#include "gcc_parse.hpp"
#include <algorithm>
#include <cstddef>
#include <format>
#include <fstream>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace rng = std::ranges;
namespace views = rng::views;

using string = std::string;
using std::operator""sv;

struct Node
{
    Node(int p_nid,
         std::string p_fn_name,
         std::string p_demangled_name,
         std::string p_visibility,
         std::string p_avaliablity,
         std::string p_flags)
      : id(p_nid)
      , fn_name(std::move(p_fn_name))
      , demangled_name(std::move(p_demangled_name))
      , visibility(std::move(p_visibility))
      , availability(std::move(p_avaliablity))
      , flags(std::move(p_flags)) {};

    Node& operator=(Node const&) = default;
    Node(Node const&) = default;
    Node& operator=(Node&&) = default;
    Node(Node&&) = default;
    // Node() = default;

    int id;
    std::string fn_name;
    std::string demangled_name;
    std::string visibility;
    std::string availability;
    std::string flags;
    std::vector<Node> callees;
    std::vector<Node> callers;
};

inline auto get_names(std::span<Node> v)
{
    return v | views::transform([](auto& n) { return n.fn_name; })
           | rng::to<std::vector<string>>();
}

template<>
struct std::formatter<Node> : std::formatter<std::string>
{
    auto format(Node& n, format_context& ctx) const
    {
        return formatter<std::string>::format(std::format("id: {}"
                                                          "func_name: {}"
                                                          "demangled_name: {}"
                                                          "visibility: {}"
                                                          "availability: {}"
                                                          "flags: {}"
                                                          "callers: {}"
                                                          "callees: {}",
                                                          n.id,
                                                          n.fn_name,
                                                          n.demangled_name,
                                                          n.visibility,
                                                          n.availability,
                                                          n.flags,
                                                          get_names(n.callers),
                                                          get_names(n.callees)),
                                              ctx);
    }
};

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

std::vector<string> parse_fn_list(std::string_view inp)
{
    auto split_vec
      = inp | views::split(' ')
        | views::transform([](auto&& s) { return std::string_view(s); })
        | rng::to<std::vector<string>>();

    std::vector<string> res;
    for (string& s : split_vec) {
        string name = *(s | views::split('/') | views::take(1)
                        | views::transform(
                          [](auto&& ss) { return trim(std::string_view(ss)); }))
                         .begin();

        res.push_back(name);
    }
    return res;
}

void parse(string file_path)
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
    // for (auto& e : raw_entries) {
    //     std::println("Entry:");
    //     for (auto&& s :
    //          e | views::split('\n') | rng::to<std::vector<string>>()) {
    //         std::println("{}", s);
    //     }
    //     std::println("\n");
    // }

    std::vector<std::unordered_map<string, string>> table_entries;
    auto fn_name_re = ctre::search<".+[0-9]+">;
    for (auto& raw_entry : raw_entries) {
        std::println("\nNEW ENTRY\n");
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

                // *sigh* C++ needs a collect method
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
    std::unordered_map<std::string_view, Node> all_nodes;
    for (auto& entry : table_entries) {
        Node n = { std::stoi(entry["id"]),  entry["fn_name"],
                   entry["demangled_name"], entry["visablity"],
                   entry["avaliablity"],    entry["function_flags"] };
        all_nodes.emplace(entry["fn_name"], n);
    }

    // Create Edges
    for (auto& entry : table_entries) {
        std::string_view name = entry["fn_name"];
        if (!all_nodes.contains(name)) {
            continue;
        }
        auto caller_strs = parse_fn_list(entry["called_by"]);
        auto callee_strs = parse_fn_list(entry["calls"]);

        auto get_node = [&all_nodes](std::string_view name) -> Node& {
            return all_nodes.at(name);
        };

        Node& n = get_node(name);
        n.callers = caller_strs | views::transform(get_node)
                    | rng::to<std::vector<Node>>();
    }
}
