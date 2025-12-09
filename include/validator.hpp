#pragma once

#include <print>
#include <span>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include "elf_parser.hpp"

namespace safe {

class Validator{
public:
    Validator(symbol_s p_sym, std::span<std::byte> p_text);
    ~Validator () {};
private:
    symbol_s m_sym;
    std::span<std::byte> m_text
    void find_throw_type(std::string_view func_name);
}
}