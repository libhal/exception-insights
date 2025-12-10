#include "validator.hpp"

namespace safe {

std::optional<std::vector<symbol_s>> Validator::find_typeinfo(
  std::string_view func_name)
{
    symbol_s func_sym = get_symbol(func_name).value();
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

    std::ofstream out("../../testing_programs/logs/function_binary.txt",
                      std::ios::app);
    out << std::format("===========================\n");
    out << std::format("Function: {}\n", demangle(func_name.data()));
    out << std::format("===========================\n");

    for (size_t i = 0; i < func_size - 8; ++i) {
        uint64_t current_addr = func_addr + i;
        int32_t rel_offset = *reinterpret_cast<const int32_t*>(func_start + i);
        uint64_t target_addr = current_addr + 4 + rel_offset;

        out << std::format(
          "Offset: {:4} | Bytes: {:02x} {:02x} {:02x} {:02x} | Target: 0x{:x}\n",
          i,
          static_cast<uint8_t>(func_start[i]),
          static_cast<uint8_t>(func_start[i + 1]),
          static_cast<uint8_t>(func_start[i + 2]),
          static_cast<uint8_t>(func_start[i + 3]),
          target_addr);

        if (rtti_sym.contains(target_addr)) {
            std::string demangled = demangle(rtti_sym[target_addr].name.data());
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
    std::ofstream out("../../testing_programs/logs/RTTI_typeinfo.txt");
    out << std::format("===================================\n");
    out << std::format("RTTI Address | Demangled Throw Name\n");
    out << std::format("===================================\n");
    for (auto& sym : m_sym) {
        std::string demangle_sym = demangle(sym.name.c_str());
        if (demangle_sym.starts_with("typeinfo")) {
            out << std::format("   {}   | {}\n", sym.value, demangle_sym);
            rtti_sym.emplace(get_symbol(sym.name).value().value, sym);
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

std::string Validator::demangle(const char* mangled)
{
    int status;
    char* demangled = abi::__cxa_demangle(mangled, nullptr, nullptr, &status);

    if (status == 0) {
        std::string result(demangled);
        std::free(demangled);
        return result;
    }

    return mangled;
}

}  // namespace safe
