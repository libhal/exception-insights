#pragma once

#include "abi_parse.hpp"  // LsdaParser, Scope, ScopeHandler, HandlerType
#include "validator.hpp"  // symbol_s, Validator

#include <cstdint>
#include <expected>
#include <string>
#include <string_view>
#include <vector>

namespace safe {

// One record per handler inside a scope.
struct CatchRecord
{
    std::string scope_id;       // example: "scope[0]"
    HandlerType kind;           // Catch / Cleanup / Filter
    std::uint64_t range_begin;  // scope.start
    std::uint64_t range_end;    // scope.end
    std::uint64_t landing_pad;  // handler.landing_pad
    std::int64_t type_index;    // handler.type_index
};

// Relation between a single thrown RTTI symbol and the handlers that can catch
// it.
struct ThrowCatchMatch
{
    symbol_s thrown;  // RTTI symbol for the thrown type
    std::vector<const CatchRecord*> handlers;  // matching catch handlers
};

class CatchValidator
{
  public:
    // errors that say why correlation between throws and catches might fail.
    enum class CorrelateError
    {
        NoTypeinfoForFunction,  // Validator has no typeinfo for this function
        NoThrownTypes,          // Function exists, but no throws recorded
        NoCatchRecords,         // No LSDA catch records matched any thrown type
        TypeResolveFailed,      // LSDA::resolve_type() failed for some index
    };

    using CorrelateResult
      = std::expected<std::vector<ThrowCatchMatch>, CorrelateError>;

    explicit CatchValidator(const LsdaParser& parser);

    const std::vector<CatchRecord>& records() const noexcept
    {
        return m_records;
    }

    // LSDA catch records from this validator
    CorrelateResult correlate_with_throws(Validator& throw_validator,
                                          std::string_view func_name) const;

    // print the correlation or a short error message.
    void print_throw_catch_report(Validator& throw_validator,
                                  std::string_view func_name) const;

    void print_records() const;

  private:
    const LsdaParser& m_lsda;
    std::vector<CatchRecord> m_records;

    static const char* handler_kind_to_string(HandlerType kind);
};

}  // namespace safe
