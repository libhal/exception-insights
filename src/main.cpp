/**
 * @file main.cpp
 * @author SAFE Group
 * @brief main file of SAFE
 * @version 0.1
 * @date 2025-07-17
 *
 * @copyright Copyright (c) 2025
 *
 */
#include <print>
#include <queue>
#include <tuple>

#include "gcc_parse.hpp"

namespace safe {
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
}  // namespace safe

int main(int argc, char** argv)
{
    std::ignore = argc;
    std::ignore = argv;
    std::println("Hello C++: {}", __cplusplus);

    auto r = safe::parse_gcc_wpa(
      "testing_programs/build/demo_class.wpa.081i.whole-program");

    bfs(&r);
    return 0;
}