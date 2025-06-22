#pragma once
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <ctll/fixed_string.hpp>
#include <ctre.hpp>
#include <ctre/wrapper.hpp>
#include <ranges>
#include <string>

namespace rng = std::ranges;
namespace views = rng::views;

using string = std::string;
using std::operator""sv;

struct Node;

void parse(std::string file_path);

constexpr bool is_word_in_str(std::string_view const word,
                              std::string_view const full_str)
{
    constexpr auto pattern = ctll::fixed_string{ "\\n| |\\t" };
    return rng::any_of(ctre::split<pattern>(full_str),
                       [word](auto&& c) { return word == c; });
}