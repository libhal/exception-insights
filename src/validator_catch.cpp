#include "validator_catch.hpp"

#include <iomanip>
#include <iostream>
#include <type_traits>
#include <variant>

namespace safe {

namespace {

// helper used internally to turn handler kind into text.
const char* handler_kind_to_string_local(HandlerType kind)
{
    switch (kind) {
        case HandlerType::Catch:
            return "Catch";
        case HandlerType::Cleanup:
            return "Cleanup";
        case HandlerType::Filter:
            return "Filter";
    }
    return "Unknown";
}

// Turn a symbol_s into a plain address so we can compare RTTI entries between
// Validator and LSDA.
std::uint64_t symbol_address(const symbol_s& s)
{
    return std::visit(
      [](auto&& v) -> std::uint64_t {
          using T = std::decay_t<decltype(v)>;
          if constexpr (std::is_pointer_v<T>) {
              return reinterpret_cast<std::uint64_t>(v);
          } else {
              return static_cast<std::uint64_t>(v);
          }
      },
      s.value);
}

}  // namespace

const char* CatchValidator::handler_kind_to_string(HandlerType kind)
{
    return handler_kind_to_string_local(kind);
}

CatchValidator::CatchValidator(const LsdaParser& parser)
  : m_lsda(parser)
{
    m_records.clear();
    const auto& scopes = parser.get_scopes();

    std::size_t idx = 0;
    for (const auto& scope : scopes) {
        std::string scope_label = "scope[" + std::to_string(idx) + ']';

        for (const auto& h : scope.handlers) {
            CatchRecord rec{};
            rec.scope_id = scope_label;
            rec.kind = h.type;  // ScopeHandler::type
            rec.range_begin = scope.start;
            rec.range_end = scope.end;
            rec.landing_pad = h.landing_pad;
            rec.type_index = h.type_index;

            m_records.push_back(rec);
        }

        ++idx;
    }
}

CatchValidator::CorrelateResult CatchValidator::correlate_with_throws(
  Validator& throw_validator,
  std::string_view func_name) const
{
    // ask the throw side Validator which RTTI objects this function throws.
    auto thrown_opt = throw_validator.find_typeinfo(func_name);
    if (!thrown_opt.has_value()) {
        // Either the function isn't known to Validator, or it couldn't compute
        // typeinfo for it.
        return std::unexpected(CorrelateError::NoTypeinfoForFunction);
    }

    const auto& thrown_vec = *thrown_opt;
    if (thrown_vec.empty()) {
        return std::unexpected(CorrelateError::NoThrownTypes);
    }

    if (m_records.empty()) {
        return std::unexpected(CorrelateError::NoCatchRecords);
    }

    std::vector<ThrowCatchMatch> result;
    result.reserve(thrown_vec.size());

    // for each thrown type, find matching LSDA catch records.
    for (const auto& t : thrown_vec) {
        ThrowCatchMatch rel{ t, {} };
        const std::uint64_t thrown_addr = symbol_address(t);

        for (const auto& rec : m_records) {
            // skip cleanups / filters / invalid indices.
            if (rec.kind != HandlerType::Catch || rec.type_index <= 0) {
                continue;
            }

            auto handler_addr_opt = m_lsda.resolve_type(rec.type_index);
            if (!handler_addr_opt.has_value()) {
                // LSDA type table looks inconsistent; surface this as an error.
                return std::unexpected(CorrelateError::TypeResolveFailed);
            }

            if (*handler_addr_opt == thrown_addr) {
                rel.handlers.push_back(&rec);
            }
        }

        result.push_back(std::move(rel));
    }

    // if none of the thrown types mapped to any handlers at all, signal to
    // callers.
    bool any_handlers = false;
    for (const auto& m : result) {
        if (!m.handlers.empty()) {
            any_handlers = true;
            break;
        }
    }
    if (!any_handlers) {
        return std::unexpected(CorrelateError::NoCatchRecords);
    }

    return result;
}

void CatchValidator::print_throw_catch_report(Validator& throw_validator,
                                              std::string_view func_name) const
{
    auto matches_or_err = correlate_with_throws(throw_validator, func_name);

    std::cout << "[SAFE] throw/catch correlation for function " << func_name
              << ":\n";

    if (!matches_or_err.has_value()) {
        switch (matches_or_err.error()) {
            case CorrelateError::NoTypeinfoForFunction:
                std::cout
                  << "  (no typeinfo found for this function in Validator)\n";
                break;
            case CorrelateError::NoThrownTypes:
                std::cout << "  (function has no recorded throw types)\n";
                break;
            case CorrelateError::NoCatchRecords:
                std::cout
                  << "  (no LSDA catch records matched any thrown type)\n";
                break;
            case CorrelateError::TypeResolveFailed:
                std::cout
                  << "  (failed to resolve at least one LSDA type index)\n";
                break;
        }
        return;
    }

    const auto& matches = matches_or_err.value();

    for (const auto& mc : matches) {
        const auto addr = symbol_address(mc.thrown);

        std::cout << "  Thrown RTTI symbol: " << mc.thrown.name << " @ 0x"
                  << std::hex << addr << std::dec << "\n";

        if (mc.handlers.empty()) {
            std::cout << "    ❌ no matching catch handlers in LSDA\n";
        } else {
            std::cout << "    ✅ handled by " << mc.handlers.size()
                      << " catch handler(s):\n";
            for (const auto* rec : mc.handlers) {
                std::cout << "      - " << rec->scope_id << " ("
                          << handler_kind_to_string(rec->kind) << ") "
                          << "range 0x" << std::hex << rec->range_begin << "-0x"
                          << rec->range_end << ", landing_pad 0x"
                          << rec->landing_pad << std::dec << ", type_index "
                          << rec->type_index << "\n";
            }
        }
    }
}

void CatchValidator::print_records() const
{
    std::cout << "\n[Catch Handler Table]\n";

    std::size_t idx = 0;
    for (const auto& rec : m_records) {
        std::cout << "  [" << idx++ << "] "
                  << "Scope: " << rec.scope_id
                  << ", Kind: " << handler_kind_to_string(rec.kind)
                  << ", Range: 0x" << std::hex << rec.range_begin << " - 0x"
                  << rec.range_end << ", LandingPad: 0x" << rec.landing_pad
                  << std::dec << ", TypeIndex: " << rec.type_index << '\n';
    }
}

}  // namespace safe
