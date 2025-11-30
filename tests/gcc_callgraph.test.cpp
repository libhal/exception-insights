/** @file gcc_callgraph.test.cpp
 * @author Madeline Schneider
 * @brief Tests for GCC WPA callgraph parser
 * @version 0.1
 * @date 2025-07-17
 *
 * @copyright Copyright (c) 2025
 */

#include <cstddef>

#include <queue>
#include <set>
#include <stack>
#include <string>
#include <string_view>
#include <vector>

#include <boost/ut.hpp>

#include "gcc_parse.hpp"

namespace safe {
namespace {
std::vector<std::string> dfs(CallGraph const& graph, CallGraphNode const& node)
{
    std::stack<CallGraphNode const*, std::vector<CallGraphNode const*>> s;
    std::unordered_map<size_t, bool> visited;
    std::vector<std::string> res;

    s.push(&node);

    while (!s.empty()) {
        CallGraphNode const* current = s.top();
        s.pop();

        if (visited[current->id]) {
            continue;
        }
        visited[current->id] = true;

        // Process the current node (for demonstration, we print its name)
        res.push_back(current->fn_name);
        for (const auto& [callee_id, _] : current->callees) {
            auto callee_node_opt = graph.get_node_from_id(callee_id);
            if (callee_node_opt) {
                s.push(callee_node_opt.value().get());
            }
        }
    }

    return res;
}

void bfs(CallGraph const& graph, CallGraphNode const* root)
{
    if (!root) {
        return;
    }

    std::queue<std::pair<CallGraphNode const*, int>> q;  // node and depth
    std::unordered_map<int, int>
      visit_count;  // track how many times we've seen each node

    q.emplace(root, 0);

    while (!q.empty()) {
        auto [n, depth] = q.front();
        q.pop();

        // Limit visits to prevent infinite loops in cyclic graphs
        if (visit_count[static_cast<int>(n->id)] >= 10) {
            continue;
        }
        visit_count[static_cast<int>(n->id)]++;

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
            q.emplace(graph.get_node_from_id(callee).value().get(), depth + 1);
        }
    }
}
}  // namespace
}  // namespace safe

boost::ut::suite<"gcc_callgraph_parser"> gcc_callgraph_parser_tests = [] {
    using namespace boost::ut;

#if defined(__unix__) || defined(__APPLE__)
    std::system("cd ../../testing_programs/ && ./generate_and_build.sh");
#elif defined(_WIN32)
    std::system("cd ../../testing_programs/ && ./generate_and_build.ps1");
#endif

    "throw_site_check"_test = []() {
        try {
            auto raw_entries = safe::parse_gcc_wpa(
              "../../testing_programs/build/multi_tu.whole-program");

            auto graph = safe::parse_gcc_callgraph(raw_entries);
            expect(!graph.m_throw_callers.empty());
        } catch (const std::exception& e) {
            std::println(stderr, "Test skipped: {}", e.what());
            expect(false);
        }
    };

    "dfs test"_test = []() {
        auto raw_entries = safe::parse_gcc_wpa(
          "../../testing_programs/build/multi_tu.whole-program");

        auto graph = safe::parse_gcc_callgraph(raw_entries);
        auto main_node_opt = graph.get_node_from_name("main");
        expect(main_node_opt.has_value());
        auto seen_nodes = safe::dfs(graph, *main_node_opt.value());
        std::set<std::string_view> expected_fn_names
          = { "main",
              "_Z3bazi",
              "_ZN1A6methodEv",
              "__cxa_allocate_exception",
              "__cxa_throw",
              "_Z3foov",
              "_ZZ3foovENKUlvE_clEv",
              "_Z3barv",
              "__builtin_eh_pointer",
              "__cxa_begin_catch",
              "__cxa_end_catch" };

        for (auto& node_str : seen_nodes) {
            if (expected_fn_names.contains(node_str)) {
                expected_fn_names.erase(node_str);
            }
        }

        expect(expected_fn_names.empty());
    };

    "bfs_example"_test = []() {
        auto raw_entries = safe::parse_gcc_wpa(
          "../../testing_programs/build/multi_tu.whole-program");

        auto graph = safe::parse_gcc_callgraph(raw_entries);
        auto main_node_opt = graph.get_node_from_name("main");
        expect(main_node_opt.has_value());

        safe::bfs(graph, main_node_opt.value().get());
    };
};
