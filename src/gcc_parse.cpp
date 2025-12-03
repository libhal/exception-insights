/** @file gcc_parse.cpp
 * @author Madeline Schneider
 * @brief GCC WPA callgraph parser implementation file
 * @version 0.1
 * @date 2025-07-17
 *
 * @copyright Copyright (c) 2025
 */

#include <algorithm>
#include <fstream>
#include <ranges>

#include "gcc_parse.hpp"

namespace safe {

std::optional<std::shared_ptr<CallGraphNode const>> CallGraph::get_node_from_id(
  size_t p_id) const
{
    try {
        return m_nodes.at(p_id);
    } catch (std::out_of_range) {
        return std::nullopt;
    }
}

std::optional<std::shared_ptr<CallGraphNode const>>
CallGraph::get_node_from_name(std::string_view p_name) const
{
    // TODO: Make more efficent
    for (auto& [id, n] : m_nodes) {
        if (p_name == n->fn_name) {
            return n;
        }
    }

    return std::nullopt;
}

std::optional<std::shared_ptr<CallGraphNode const>> CallGraphNode::from_id(
  size_t p_id) const
{
    return m_graph.get_node_from_id(p_id);
}

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

[[maybe_unused]] std::vector<std::pair<string, std::vector<string>>>
parse_fn_list(std::string& inp)
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
    std::optional<std::pair<string, std::vector<string>>> prev = std::nullopt;
    for (string& s : split_vec) {
        if (ctre::match<"(\\(.+\\))+">(s)) {
            if (s.empty()) {
                continue;
            }

            if (!prev.has_value()) {
                throw std::invalid_argument("Invalid format given");
            }
            prev->second.emplace_back(s.data() + 1, s.length() - 2);
            continue;
        }

        string name = *(s | views::split('/') | views::drop(1) | views::take(1)
                        | views::transform(
                          [](auto&& ss) { return trim(std::string_view(ss)); }))
                         .begin();

        std::pair<string, std::vector<string>> p(name, {});
        res.push_back(p);
        prev = res.back();
    }

    return res;
}

}  // namespace

std::vector<std::unordered_map<std::string, std::string>> parse_gcc_wpa(
  std::string_view file_path)
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

        // TODO: Use rng::enumerate when clang is updated
        for (size_t i = 0; i < split_vec.size(); i++) {
            auto& entry = split_vec[i];
            if (i == 0) {
                auto m = fn_name_re.search(entry);

                auto iter = m.get<0>().to_view() | views::split('/')
                            | views::transform([](auto&& s) {
                                  return trim(std::string_view(s));
                              });

                auto fn_name = *iter.begin();
                parsed_entry["fn_name"] = fn_name;
                parsed_entry["id"] = *(iter | views::drop(1)).begin();
                auto demangle_name_iter
                  = ctre::search<"\\(.*\\)">(entry).get<0>().to_view()
                    | views::drop(1) | views::reverse | views::drop(1)
                    | views::reverse;

                parsed_entry["demangled_name"] = string(
                  demangle_name_iter.begin(), demangle_name_iter.end());

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
            parsed_entry[key] = value;
        }
        table_entries.push_back(parsed_entry);
    }

    return table_entries;
}

// Can throw: (https://en.cppreference.com/w/cpp/string/basic_string/stoul),
// bad_optional_access, should never be null
CallGraph parse_gcc_callgraph(
  std::vector<std::unordered_map<std::string, std::string>> p_parsed_table)
{
    // Create all nodes
    CallGraph graph;
    auto& table_entries = p_parsed_table;
    auto& all_nodes = graph.m_nodes;
    for (auto& entry : table_entries) {
        // Convert string id to numeric key before emplacing.
        size_t uid = static_cast<size_t>(std::stoul(entry["id"]));
        auto n = std::make_shared<CallGraphNode>(
          NodeArgs{ .p_nid = static_cast<size_t>(uid),
                    .p_fn_name = entry["fn_name"],
                    .p_demangled_name = entry["demangled_name"],
                    .p_visibility = entry["visablity"],
                    .p_avaliablity = entry["avaliablity"],
                    .p_flags = entry["function_flags"],
                    .p_graph = graph });

        all_nodes.emplace(uid, n);
    }

    // Create Edges
    for (auto& entry : table_entries) {
        size_t id = std::stoi(entry["id"]);
        if (!all_nodes.contains(id)) {
            continue;
        }
        auto caller_pairs = parse_fn_list(entry["called_by"]);
        auto callee_pairs = parse_fn_list(entry["calls"]);

        auto n = all_nodes.at(id);

        // Callers
        for (const auto& [node_id_str, attribs] : caller_pairs) {
            size_t const node_id = std::stoul(node_id_str);
            auto nn = graph.get_node_from_id(node_id).value();
            n->callers.emplace_back(nn->id, attribs);
        }

        // Callees
        for (const auto& [node_id_str, attribs] : callee_pairs) {
            size_t const node_id = std::stoul(node_id_str);
            auto nn = graph.get_node_from_id(node_id).value();
            n->callees.emplace_back(nn->id, attribs);

            if (nn->fn_name == "__cxa_throw") {
                graph.m_throw_callers.emplace_back(n);
            }
        }
    }

    return graph;
}

}  // namespace safe
