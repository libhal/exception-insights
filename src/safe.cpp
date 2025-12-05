#include "safe.hpp"

Safe::Safe(int argc, char* argv[])
{
    auto arg = get_args(argc, argv);
    if (!arg.has_value()) {
        std::print("Error: {}\n", arg.error().e_string);
        throw arg.error();
        m_last_error = arg.error();
        exit(EXIT_FAILURE);
    }

    auto elf = get_elf();
    if (!elf.has_value()) {
        std::print("Error: {}\n", elf.error().e_string);
        throw elf.error();
        m_last_error = arg.error();
        exit(EXIT_FAILURE);
    }
}

std::expected<void, error_log_s> Safe::get_args(int argc, char* argv[])
{
    if (argc == 3) {
        m_flag = std::string(argv[1]);
        m_file_name = std::string(argv[2]);
        if (m_flag != "-v") {
            return std::unexpected(
              error_log_s{ safe_error::INVALID_FLAG, "Invalid flag.\n" });
        }
        if (!std::filesystem::exists(m_file_name)) {
            return std::unexpected(error_log_s{
              safe_error::FILE_NOT_FOUND,
              std::format("File not found.\n FILE: {}\n", m_file_name) });
        }
        return {};
    } else if (argc == 2) {
        m_file_name = std::string(argv[1]);
        if (!std::filesystem::exists(m_file_name)) {
            return std::unexpected(error_log_s{
              safe_error::FILE_NOT_FOUND,
              std::format("File not found.\n FILE: {}\n", m_file_name) });
        }
        return {};
    } else {
        return std::unexpected(error_log_s{ safe_error::INVALID_ARG_AMOUNT,
                                            "Invalid argument amount.\n" });
    }
}

std::expected<void, error_log_s> Safe::get_elf()
{
    ElfParser elf(m_file_name);

    auto sym = elf.get_symbol_table();
    if (!sym.has_value()) {
        return std::unexpected(
          error_log_s{ sym.error(), "Empty symbol table" });
    }
    m_sym = sym.value();

    auto lsda = elf.get_section(".gcc_except_table");
    if (lsda.error() == elf_parser_error::EMPTY_SECTION) {
        return std::unexpected(
          error_log_s{ lsda.error(), "Empty Section Vector" });
    } else if (lsda.error() == elf_parser_error::SECTION_NOT_FOUND) {
        return std::unexpected(
          error_log_s{ lsda.error(), "Section not found: .gcc_except_table" });
    } else {
        m_lsda = lsda.value().data;
    }

    auto text = elf.get_section(".text");
    if (text.error() == elf_parser_error::EMPTY_SECTION) {
        return std::unexpected(
          error_log_s{ text.error(), "Empty Section Vector" });
    } else if (text.error() == elf_parser_error::SECTION_NOT_FOUND) {
        return std::unexpected(
          error_log_s{ text.error(), "Section not found: .text" });
    } else {
        m_text = text.value().data;
    }
    return {};
}
