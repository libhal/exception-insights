#pragma once
#include <cxxabi.h>

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <print>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <format>
#include <filesystem>

#include "elf_parser.hpp"
#include "gelf.h"

namespace safe {

class Validator
{
  public:
    Validator(std::span<symbol_s> p_sym, section_s p_text)
      : m_sym(p_sym)
      , m_text(std::move(p_text))
    {
        std::filesystem::create_directories("../logs");
        std::ofstream out("../logs/function_binary.txt");
        out.close();
        collect_rtti_sym();
    }
    ~Validator() = default;
    std::optional<std::vector<symbol_s>> find_typeinfo(
      std::string_view func_name);
    std::optional<std::string> demangle(const char* mangled);
    std::optional<symbol_s> get_symbol(std::string_view name);

  private:
    std::span<symbol_s> m_sym;
    section_s m_text;
    std::unordered_map<uint64_t, symbol_s> rtti_sym;

    void collect_rtti_sym();
};
}  // namespace safe
