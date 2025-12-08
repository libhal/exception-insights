/**
 * @file elf_parser.cpp
 * @author Michael Chan
 * @brief ELF file parser implementation file
 * @version 0.1
 * @date 2025-07-17
 *
 * @copyright Copyright (c) 2025
 *
 **/

#include "elf_parser.hpp"
#include <cstddef>
#include <system_error>

ElfParser::ElfParser(std::string_view p_file_name)
  : m_file_name(p_file_name)
{
    try {
        if (elf_version(EV_CURRENT) == EV_NONE) {
            throw std::runtime_error(
              "ELF library initialization failed. Wrong ELF format version.");
        }
    } catch (const std::runtime_error& e) {
        std::println(stderr, "Error opening {}: {}", m_file_name, e.what());
        exit(EXIT_FAILURE);
    }

    m_file = open(m_file_name.c_str(), O_RDONLY, 0);
    try {
        if (m_file < 0) {
            throw std::system_error(
              errno, std::generic_category(), "Open failed.");
        }
    } catch (const std::system_error& e) {
        std::println(
          stderr, "Error opening file: {}\n{}", m_file_name, e.what());
        exit(EXIT_FAILURE);
    }

    m_elf = elf_begin(m_file, ELF_C_READ, NULL);
    try {
        if (m_elf == NULL) {
            throw std::runtime_error("ELF_begin failed.");
        }
        if (elf_kind(m_elf) != ELF_K_ELF) {
            throw std::runtime_error("Not a ELF object.");
        }
    } catch (const std::runtime_error& e) {
        std::println(stderr, "Error opening {}: {}", m_file_name, e.what());
        exit(EXIT_FAILURE);
    }

    std::println("{} {}-bit ELF object\n",
                 m_file_name,
                 m_elf_class == ELFCLASS32 ? 32 : 64);

    m_load_elf_header();
    m_load_section_header();
    m_load_program_header();
    m_load_symbol_table();
};

ElfParser::~ElfParser()
{
    elf_end(m_elf);
    close(m_file);
    std::println("ELF file closed.");
}

void ElfParser::m_load_elf_header()
{
    if ((gelf_getehdr(m_elf, &m_elf_header)) == NULL) {
        std::println(stderr,
                     "Error (loadElfHeader):Elf header failed to load: {}.",
                     elf_errmsg(-1));
        m_elf_header_loaded = false;
    } else {
        m_elf_header_loaded = true;
    }
}

void ElfParser::m_load_section_header()
{
    if (!m_elf_header_loaded) {
        std::println(stderr,
                     "Error (loadSectionHeader): Elf header was not previously "
                     "loaded: {}.",
                     elf_errmsg(-1));
    }

    section_s current_section;
    char* section_name;
    Elf_Scn* section = nullptr;
    GElf_Shdr current_section_header;
    while ((section = elf_nextscn(m_elf, section)) != NULL) {
        if (gelf_getshdr(section, &current_section_header)
            != &current_section_header) {
            std::println(
              stderr,
              "Error (load_section_header): Unable to get section header: {}.",
              elf_errmsg(-1));
            continue;
        }
        section_name = elf_strptr(
          m_elf, m_elf_header.e_shstrndx, current_section_header.sh_name);
        if (section_name == nullptr) {
            std::println(
              stderr,
              "Error (load_section_header): Unable to get section name: {}.",
              elf_errmsg(-1));
            continue;
        }

        // m_section_header.emplace(section_name, current_section_header);
        current_section.header = current_section_header;

        Elf_Data* section_data = elf_getdata(section, nullptr);
        if (!section_data) {
            std::println(
              stderr,
              "Error (load_section_header): Unable to get section {} data: {}.",
              section_name,
              elf_errmsg(-1));
            continue;
        }

        std::vector<std::byte> current_section_data;
        if (current_section_header.sh_type == SHT_NOBITS) {
            current_section_data = {};
        } else {
            std::vector<std::byte> parsed_data(
              reinterpret_cast<const std::byte*>(section_data->d_buf),
              reinterpret_cast<const std::byte*>(section_data->d_buf)
                + section_data->d_size);
            current_section_data = parsed_data;
        }
        current_section.data = current_section_data;

        m_sections.emplace(section_name, current_section);
    }
}

void ElfParser::m_load_symbol_table()
{
    if (m_sections.find(".symtab") == m_sections.end()) {
        return;
    }

    if (m_sections.find(".strtab") == m_sections.end()) {
        return;
    }

    const GElf_Shdr& symtab_hdr = m_sections[".symtab"].header;
    const std::vector<std::byte>& symtab_data = m_sections[".symtab"].data;
    const std::vector<std::byte>& strtab_data = m_sections[".strtab"].data;
    size_t symtab_count = symtab_hdr.sh_size / symtab_hdr.sh_entsize;

    for (size_t i = 0; i < symtab_count; i++) {
        const GElf_Sym* sym = reinterpret_cast<const GElf_Sym*>(
          symtab_data.data() + (i * symtab_hdr.sh_entsize));

        const char* name
          = reinterpret_cast<const char*>(strtab_data.data() + sym->st_name);

        if (sym->st_name == 0) {
            name = "";
        }

        symbol_s symbol = { name,         sym->st_value, sym->st_size,
                            sym->st_info, sym->st_other, sym->st_shndx };
        m_symbol_table.emplace_back(symbol);
    }
}

void ElfParser::m_load_program_header()
{
    if (!m_elf_header_loaded) {
        std::println(
          stderr,
          "Error (Load_program_header): Elf header was not previously "
          "loaded: {}.",
          elf_errmsg(-1));
    } else {
        GElf_Phdr current_program_header;
        for (int i = 0; i < m_elf_header.e_phnum; i++) {
            if (gelf_getphdr(m_elf, i, &current_program_header)
                != &current_program_header) {
                std::println(
                  stderr,
                  "Error (Load_program_header): Unable to get program "
                  "header: {}.",
                  elf_errmsg(-1));
                continue;
            }
            m_program_header.emplace_back(current_program_header);
        }
    }
}

std::expected<GElf_Ehdr, elf_parser_error> ElfParser::get_elf_header()
{
    if (!m_elf_header_loaded) {
        return std::unexpected(elf_parser_error::UNLOADED_ELF_HEADER);
    } else {
        return m_elf_header;
    }
}

std::expected<section_s, elf_parser_error> ElfParser::get_section(
  std::string_view p_section)
{
    if (m_sections.empty()) {
        return std::unexpected(elf_parser_error::EMPTY_SECTION);
    }

    if (!m_sections.contains(p_section)) {
        return std::unexpected(elf_parser_error::SECTION_NOT_FOUND);
    }

    return m_sections[p_section];
}

std::expected<std::span<GElf_Phdr>, elf_parser_error>
ElfParser::get_program_header()
{
    if (m_program_header.empty()) {
        return std::unexpected(elf_parser_error::EMPTY_PROGRAM);
    }
    return m_program_header;
}

std::expected<std::span<symbol_s>, elf_parser_error>
ElfParser::get_symbol_table()
{
    if (m_symbol_table.empty()) {
        return std::unexpected(elf_parser_error::EMPTY_SYMBOL);
    }
    return m_symbol_table;
}
