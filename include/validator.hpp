#pragma once
#include <cxxabi.h>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <optional>
#include <print>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "abi_parse.hpp"
#include "elf_parser.hpp"
#include "gelf.h"

namespace safe {
struct CatchRecord
{
    std::string scope_id;       // example: "scope[0]"
    HandlerType kind;           // Catch / Cleanup / Filter
    std::uint64_t range_begin;  // scope.start
    std::uint64_t range_end;    // scope.end
    std::uint64_t landing_pad;  // handler.landing_pad
    std::int64_t type_index;    // handler.type_index
};

struct ThrowCatchMatch
{
    symbol_s thrown;                          // RTTI symbol for the thrown type
    std::vector<const CatchRecord*> handlers; // matching catch handlers
};

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
    std::optional<std::vector<symbol_s>> find_typeinfo(std::string_view func_name);
    std::optional<std::string> demangle(const char* mangled);
    std::optional<symbol_s> get_symbol(std::string_view name);

    enum class CorrelateError
    {
        NoTypeinfoForFunction,  // Validator has no typeinfo for this function
        NoThrownTypes,          // Function exists, but no throws recorded
        NoCatchRecords,         // No LSDA catch records matched any thrown type
        TypeResolveFailed,      // LSDA::resolve_type() failed for some index
        NoLsdaLoaded,           // load_lsda() never called
    };

    using Result = std::expected<std::vector<ThrowCatchMatch>, CorrelateError>;

    void load_lsda(const LsdaParser& lsda);
    Result analyze_exceptions(std::string_view func_name) const;

    const std::vector<CatchRecord>& records() const noexcept { return m_records; }

  private:
    std::span<symbol_s> m_sym;
    section_s m_text;
    std::unordered_map<std::uint64_t, symbol_s> rtti_sym;
    const LsdaParser* m_lsda = nullptr;   
    std::vector<CatchRecord> m_records;   // flattened handler table

    void collect_rtti_sym();
};

}  // namespace safe
