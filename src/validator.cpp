#include "validator.hpp"

namespace safe {

std::optional<std::vector<symbol_s>> Validator::find_typeinfo(
  std::string_view func_name)
{
    auto func_sym_opt = get_symbol(func_name);
    if (!func_sym_opt.has_value()) {
        return std::nullopt;
    }

    symbol_s func_sym = *func_sym_opt;
    uint64_t func_addr = func_sym.value;
    GElf_Shdr text_hdr = m_text.header;
    uint64_t text_addr = text_hdr.sh_addr;
    uint64_t offset = func_addr - text_addr;

    if (offset >= m_text.data.size()) {
        std::println("Error: function address out of .text bounds");
        return std::nullopt;
    }

    const std::byte* func_start = m_text.data.data() + offset;
    size_t func_size = func_sym.size;

    std::vector<symbol_s> thrown_obj;

    std::ofstream out("../logs/function_binary.txt", std::ios::app);
    out << std::format("===========================\n");
    out << std::format("Function: {}\n",
                       demangle(func_name.data()).value_or(func_name.data()));
    out << std::format("===========================\n");

    // safer than (func_size - 8)
    for (size_t i = 0; i + 4 <= func_size; ++i) {
        uint64_t current_addr = func_addr + i;
        int32_t rel_offset = *reinterpret_cast<const int32_t*>(func_start + i);
        uint64_t target_addr = current_addr + 4 + static_cast<int64_t>(rel_offset);

        out << std::format("Offset: {:4} | Bytes: {:02x} {:02x} {:02x} {:02x} "
                           "| Target: 0x{:x}\n",
                           i,
                           static_cast<uint8_t>(func_start[i]),
                           static_cast<uint8_t>(func_start[i + 1]),
                           static_cast<uint8_t>(func_start[i + 2]),
                           static_cast<uint8_t>(func_start[i + 3]),
                           target_addr);

        if (rtti_sym.contains(target_addr)) {
            std::string demangled
              = demangle(rtti_sym[target_addr].name.data())
                  .value_or(rtti_sym[target_addr].name.data());
            out << std::format(
              "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"
              "^^^^^ Throw Found: {}\n",
              demangled);
            thrown_obj.emplace_back(rtti_sym[target_addr]);
        }
    }
    out.close();
    return thrown_obj;
}

void Validator::collect_rtti_sym()
{
    std::ofstream out("../logs/RTTI_typeinfo.txt");
    out << std::format("===================================\n");
    out << std::format("RTTI Address | Demangled Throw Name\n");
    out << std::format("===================================\n");
    for (auto& sym : m_sym) {
        auto demangle_sym = demangle(sym.name.c_str());
        if (!demangle_sym) {
            continue;
        }
        if (demangle_sym->starts_with("typeinfo")) {
            out << std::format("   {}   | {}\n", sym.value, demangle_sym.value());
            rtti_sym.emplace(sym.value, sym);
        }
    }
    out.close();
}

std::optional<symbol_s> Validator::get_symbol(std::string_view name)
{
    for (const auto& s : m_sym) {
        if (s.name == name) {
            return s;
        }
    }
    return std::nullopt;
}

std::optional<std::string> Validator::demangle(const char* mangled)
{
    int status = 0;
    char* demangled = abi::__cxa_demangle(mangled, nullptr, nullptr, &status);

    if (status == 0 && demangled) {
        std::string result(demangled);
        std::free(demangled);
        return result;
    }
    return std::nullopt;
}

void Validator::load_lsda(const LsdaParser& lsda)
{
    std::println("[Validator] load_lsda: begin");
    m_lsda = &lsda;
    m_records.clear();

    const auto& scopes = lsda.get_scopes();
    std::println("[Validator] load_lsda: scopes = {}", scopes.size());
    std::size_t idx = 0;

    for (const auto& scope : scopes) {
        std::string scope_label = "scope[" + std::to_string(idx) + ']';
        for (const auto& h : scope.handlers) {
            CatchRecord rec{};
            rec.scope_id    = scope_label;
            rec.kind        = h.type;
            rec.range_begin = scope.start;
            rec.range_end   = scope.end;
            rec.landing_pad = h.landing_pad;
            rec.type_index  = h.type_index;
            m_records.push_back(std::move(rec));
        }
        ++idx;
    }
    std::println("[Validator] load_lsda: records = {}", m_records.size());
}

Validator::Result Validator::analyze_exceptions(std::string_view func_name) const
{
    if (m_lsda == nullptr) {
        return std::unexpected(CorrelateError::NoLsdaLoaded);
    }

    // Use teammate's throw info as the source of truth
    auto thrown_opt = const_cast<Validator*>(this)->find_typeinfo(func_name);
    if (!thrown_opt.has_value()) {
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

    for (const auto& t : thrown_vec) {
        ThrowCatchMatch rel{ t, {} };
        const std::uint64_t thrown_addr = t.value;

        for (const auto& rec : m_records) {
            if (rec.type_index == 0) {
                rel.handlers.push_back(&rec);
                continue;
            }
            if (rec.type_index < 0) {
                continue;
            }
            if (rec.kind != HandlerType::Catch) {
                continue;
            }

            auto handler_addr_opt = m_lsda->resolve_type(rec.type_index);
            if (!handler_addr_opt.has_value()) {
                continue; 
            }

            if (*handler_addr_opt == thrown_addr) {
                rel.handlers.push_back(&rec);
            }
        }

        result.push_back(std::move(rel));
    }

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

}  // namespace safe
