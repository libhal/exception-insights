#pragma once
#include <cctype>
#include <cstddef>
#include <ctll/fixed_string.hpp>
#include <ctre.hpp>
#include <ctre/wrapper.hpp>
#include <format>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace rng = std::ranges;
namespace views = rng::views;

using string = std::string;
using std::operator""sv;
using usize = size_t;

void parse(std::string_view file_path);
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
      , flags(std::move(p_flags)){};

    Node& operator=(Node const&) = default;
    Node(Node const&) = default;
    Node& operator=(Node&&) = default;
    Node(Node&&) = default;

    int id;
    std::string fn_name;
    std::string demangled_name;
    std::string visibility;
    std::string availability;
    std::string flags;
    std::vector<std::pair<usize, std::vector<string>>> callees;
    std::vector<std::pair<usize, std::vector<string>>> callers;

    friend void parse(std::string_view file_path);

    // Return a reference to the node with the given id. Use `.at` so we do
    // not implicitly default-construct a Node when the id is missing.
    // Note: this throws std::out_of_range if id is not present.
    static Node& get_node_from_id(usize id)
    {
        return all_nodes.at(id);
    }

    // TODO: Make more efficent
    static std::optional<std::reference_wrapper<Node>> get_node_from_name(
      std::string_view name)
    {
        for (auto [id, n] : all_nodes) {
            if (name == n.fn_name) {
                return std::ref(n);
            }
        }

        return std::nullopt;
    }

  private:
    static inline std::unordered_map<usize, Node> all_nodes;
};

constexpr inline auto get_names(
  std::span<const std::pair<usize, std::vector<string>>> v)
{
    return v | views::transform([](const auto& p) {
               return std::format(
                 "{}: {}", Node::get_node_from_id(p.first).fn_name, p.second);
           })
           | rng::to<std::vector<string>>();
}

template<>
struct std::formatter<Node> : std::formatter<std::string>
{
    auto format(const Node& n, format_context& ctx) const
    {
        return formatter<std::string>::format(std::format("id: {}\n"
                                                          "func_name: {}\n"
                                                          "demangled_name: {}\n"
                                                          "visibility: {}\n"
                                                          "availability: {}\n"
                                                          "flags: {}\n"
                                                          "callers: {}\n"
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