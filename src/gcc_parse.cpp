#include <algorithm>
#include <cctype>
#include <cstddef>
#include <format>
#include <fstream>
#include <optional>
#include <print>
#include <queue>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "gcc_parse.hpp"

namespace safe {
namespace {
using string = std::string;
using std::operator""sv;
namespace rng = std::ranges;
namespace views = rng::views;

constexpr string trim(std::string_view str)
{
    auto trimmed = str
                   | views::drop_while([](char c) { return std::isspace(c); })
                   | views::reverse
                   | views::drop_while([](char c) { return std::isspace(c); })
                   | views::reverse;

    return { trimmed.begin(), trimmed.end() };
}

constexpr bool is_whitespace(std::string_view sv)
{

    return rng::all_of(sv, [](char c) { return std::isspace(c); });
}

constexpr bool is_word_in_str(std::string_view const word,
                              std::string_view const full_str)
{
    constexpr auto pattern = ctll::fixed_string{ "\\n| |\\t" };
    return rng::any_of(ctre::split<pattern>(full_str),
                       [word](auto&& c) { return word == c; });
}

std::vector<std::pair<string, std::vector<string>>> parse_fn_list(
  std::string& inp)
{
    std::vector<string> split_vec;
    bool is_attr_str = false;

    string buf = "";
    for (size_t pos = 0; pos < inp.length(); pos++) {
        const auto c = inp[pos];
        if (buf.empty() && c == ' ') {
            continue;
        }
        if (c == '(' && !is_attr_str && buf.empty()) {
            buf.push_back(c);
            is_attr_str = true;
            continue;
        }

        if ((c == ')' && is_attr_str) || (!is_attr_str && c == ' ')) {
            if (c != ' ') {
                buf.push_back(c);
            }

            split_vec.push_back(buf);
            buf = "";
            is_attr_str = false;
            continue;
        }

        if (is_attr_str || c != ' ') {
            buf.push_back(c);
        }
    }

    if (!buf.empty()) {
        split_vec.push_back(buf);
    }

    std::vector<std::pair<string, std::vector<string>>> res;
    std::pair<string, std::vector<string>>* prev = nullptr;
    for (string& s : split_vec) {
        if (ctre::match<"(\\(.+\\))+">(s)) {
            if (s.empty()) {
                continue;
            }

            if (!prev) {
                throw std::invalid_argument("Invalid format given");
            }
            prev->second.emplace_back(s.data() + 1, s.length() - 2);
            continue;
        }

        string name = *(s | views::split('/') | views::take(1)
                        | views::transform(
                          [](auto&& ss) { return trim(std::string_view(ss)); }))
                         .begin();

        std::pair<string, std::vector<string>> p(name, {});
        res.push_back(p);
        prev = &res.back();
    }

    return res;
}
}  // namespace

void bfs(CallGraphNode* root)
{
    if (!root) {
        return;
    }

    std::queue<std::pair<CallGraphNode*, int>> q;  // node and depth
    std::unordered_map<int, int>
      visit_count;  // track how many times we've seen each node

    q.emplace(root, 0);

    while (!q.empty()) {
        auto [n, depth] = q.front();
        q.pop();

        // Limit visits to prevent infinite loops in cyclic graphs
        if (visit_count[n->id] >= 10) {
            continue;
        }
        visit_count[n->id]++;

        // Print current node
        std::string indent(static_cast<size_t>(depth) * 2, ' ');
        std::println(
          "{}[{}] {} ({})", indent, n->id, n->fn_name, n->demangled_name);

        // Enqueue all callees
        for (auto& [callee, attrs] : n->callees) {
            if (!attrs.empty()) {
                std::println("{}  with attributes: {}", indent, attrs);
            }
            // `callee` is an id (size_t). Enqueue the address of the
            // corresponding Node instance.
            q.emplace(&CallGraphNode::get_node_from_id(callee).value().get(),
                      depth + 1);
        }
    }
}

CallGraphNode parse_gcc_wpa(std::string_view file_path)
{
    std::ifstream file;
    file.open(file_path);
    if (!file.is_open()) {
        throw std::runtime_error(
          std::format("Cannot open file: {}", file_path));
    }

    string line;
    while (std::getline(file, line)) {
        if (rng::equal(line, std::string("Symbol table:"))) {
            break;
        }
    }

    bool in_entry = false;
    bool is_function = true;
    std::vector<string> raw_entries;
    std::string cur_raw_entry = "";
    while (std::getline(file, line)) {
        if (line.length() == 0) {
            continue;
        }

        if (!std::isspace(line[0])) {
            in_entry = false;
        }

        auto trimmed_line = trim(line);

        if (trimmed_line.starts_with("Type")
            && !is_word_in_str("function", trimmed_line)) {
            is_function = false;
            continue;
        }

        if (!in_entry) {
            if (is_function && cur_raw_entry.length() != 0) {
                raw_entries.push_back(cur_raw_entry);
            }
            cur_raw_entry = "";
            is_function = true;  // We assume the next entry will be a function
            in_entry = true;
        }

        cur_raw_entry += trimmed_line + "\n";
    }

    if (is_function) {
        raw_entries.push_back(cur_raw_entry);
    }

    file.close();

    std::vector<std::unordered_map<string, string>> table_entries;
    auto fn_name_re = ctre::search<".+[0-9]+">;
    for (auto& raw_entry : raw_entries) {
        if (raw_entry.length() == 0 || is_whitespace(raw_entry)) {
            continue;
        }
        std::unordered_map<string, string> parsed_entry;
        std::vector<string> split_vec
          = raw_entry | views::split('\n') | views::transform([](auto&& s) {
                return trim(std::string_view(s));
            })
            | views::filter([](auto&& s) { return s.length() != 0; })
            | rng::to<std::vector<string>>();

        // FIXME: Hacky solution to remove personality functions, in future we
        // should ignore all functions that are disjoint with the callgraph.
        auto first_line = split_vec[0];
        if (!rng::search(first_line, "__gxx_personality"sv).empty()) {
            continue;
        }
        std::println("\nNEW ENTRY\n");
        // TODO: Use rng::enumerate when clang is updated
        for (size_t i = 0; i < split_vec.size(); i++) {
            auto& entry = split_vec[i];
            if (i == 0) {
                auto m = fn_name_re.search(entry);
                // std::println("entry:{}", entry);

                auto iter = m.get<0>().to_view() | views::split('/')
                            | views::transform([](auto&& s) {
                                  return trim(std::string_view(s));
                              });
                // Find a better way to do this, std::next nor std::advanced
                // work as well nor did it++
                auto fn_name = *iter.begin();
                parsed_entry["fn_name"] = fn_name;
                parsed_entry["id"] = *(iter | views::drop(1)).begin();
                auto demangle_name_iter
                  = ctre::search<"\\(.*\\)">(entry).get<0>().to_view()
                    | views::drop(1) | views::reverse | views::drop(1)
                    | views::reverse;

                parsed_entry["demangled_name"] = string(
                  demangle_name_iter.begin(), demangle_name_iter.end());

                std::println("fn_name: {}, id: {}, demangled_name: {}",
                             parsed_entry["fn_name"],
                             parsed_entry["id"],
                             parsed_entry["demangled_name"]);
                continue;
            }

            // TODO: Track misc strings
            auto colon_match = ctre::split<":">(entry);
            auto collect_colon_split
              = colon_match | views::transform([](auto&& s) {
                    return trim(std::string_view(s));
                })
                | views::filter([](auto&& s) { return s.length() != 0; })
                | rng::to<std::vector<string>>();

            auto kv_iter = colon_match | views::transform([](auto&& s) {
                               return string(s);
                           });
            string key = *kv_iter.begin() | views::transform([](char& c) {
                return static_cast<char>(std::tolower(c));
            }) | rng::to<string>();
            rng::replace(key, ' ', '_');
            string value = trim(*(kv_iter | views::drop(1)).begin());
            std::println("{}: {}", key, value);
            parsed_entry[key] = value;
        }
        table_entries.push_back(parsed_entry);
    }

    // Create all nodes
    auto& all_nodes = CallGraphNode::all_nodes;
    for (auto& entry : table_entries) {
        // Convert string id to numeric key before emplacing.
        size_t uid = static_cast<size_t>(std::stoul(entry["id"]));
        CallGraphNode n = { static_cast<int>(uid),   entry["fn_name"],
                            entry["demangled_name"], entry["visablity"],
                            entry["avaliablity"],    entry["function_flags"] };
        all_nodes.emplace(uid, std::move(n));
    }

    // Create Edges
    for (auto& entry : table_entries) {
        size_t id = std::stoi(entry["id"]);
        if (entry["demangled_name"] == "bar") {
            std::println("Breakpoint");
        }
        if (!all_nodes.contains(id)) {
            continue;
        }
        auto caller_pairs = parse_fn_list(entry["called_by"]);
        auto callee_pairs = parse_fn_list(entry["calls"]);

        CallGraphNode& n = all_nodes.at(id);

        // Callers
        for (const auto& [node_name, attribs] : caller_pairs) {
            auto& nn
              = CallGraphNode::get_node_from_name(node_name).value().get();
            n.callers.emplace_back(nn.id, attribs);
        }

        for (const auto& [node_name, attribs] : callee_pairs) {
            auto& nn
              = CallGraphNode::get_node_from_name(node_name).value().get();
            n.callees.emplace_back(nn.id, attribs);
        }
    }

    std::println("Nodes:\n");
    CallGraphNode& main
      = CallGraphNode::get_node_from_name("main").value().get();

    // Show BFS
    return main;
}

}  // namespace safe