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

#include "../include/elf_parser.hpp"

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
        std::println(stderr, "Error opening {}: {}", m_file_name, e.what());
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
};

ElfParser::~ElfParser()
{
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

    char* section_name;
    Elf_Scn* section = nullptr;
    GElf_Shdr current_section_header;
    while ((section = elf_nextscn(m_elf, section)) != NULL) {
        if (gelf_getshdr(section, &current_section_header) !=
            &current_section_header) {
            std::println(
              stderr,
              "Error (load_section_header): Unable to get section header: {}.",
              elf_errmsg(-1));
            continue;
        }

        if ((section_name = elf_strptr(m_elf,
                                       m_elf_header.e_shstrndx,
                                       current_section_header.sh_name)) ==
            NULL) {
            std::println(
              stderr,
              "Error (load_section_header): Unable to get section name: {}.",
              elf_errmsg(-1));
            continue;
        }

        m_section_header.emplace(section_name, current_section_header);

        Elf_Data* section_data = elf_getdata(section, nullptr);
        if (!section_data) {
            std::println(
              stderr,
              "Error (load_section_header): Unable to get section {} data: {}.",
              section_name,
              elf_errmsg(-1));
            continue;
        }

        if (current_section_header.sh_type == SHT_NOBITS) {
            m_section_data.emplace(section_name, std::vector<std::byte>());
        } else {
            std::vector<std::byte> parsed_data(
              reinterpret_cast<const std::byte*>(section_data->d_buf),
              reinterpret_cast<const std::byte*>(section_data->d_buf) +
                section_data->d_size);
            m_section_data.emplace(section_name, parsed_data);
        }
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
            if (gelf_getphdr(m_elf, i, &current_program_header) !=
                &current_program_header) {
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

void ElfParser::print_elf_header()
{
    if (!m_elf_header_loaded) {
        std::println(
          stderr,
          "Error (print_elf_header): Elf header was not previously loaded: {}.",
          elf_errmsg(-1));
    } else {
        std::println("ELF Header");
        std::println("====================");
        std::print("ident: ");
        for (int i = 0; i < 16; i++) {
            std::print("0x{:X}, ", m_elf_header.e_ident[i]);
        }
        std::println("");
        std::println("type: 0x{:X}", m_elf_header.e_type);
        std::println("machine: 0x{:X}", m_elf_header.e_machine);
        std::println("version: 0x{:X}", m_elf_header.e_version);
        std::println("entry: 0x{:X}", m_elf_header.e_entry);
        std::println("phoff: 0x{:X}", m_elf_header.e_phoff);
        std::println("shoff: 0x{:X}", m_elf_header.e_shoff);
        std::println("flags: 0x{:X}", m_elf_header.e_flags);
        std::println("ehsize: 0x{:X}", m_elf_header.e_ehsize);
        std::println("phentsize: 0x{:X}", m_elf_header.e_phentsize);
        std::println("shentsize: 0x{:X}", m_elf_header.e_shentsize);
        std::println("shnum: 0x{:X}", m_elf_header.e_shnum);
        std::println("phnum: 0x{:X}", m_elf_header.e_phnum);
    }
}

void ElfParser::print_section_header()
{
    if (m_section_header.empty()) {
        std::println(stderr,
                     "Error(print_section_header): No section headers exists.");
    } else {
        std::println("Section Header: ({})", m_elf_header.e_shnum);
        std::println("====================");
        for (const auto& section : m_section_header) {
            std::println("Section: {}", section.first);
            std::println("   type : 0x{:X}", section.second.sh_type);
            std::println("   flags : 0x{:X}", section.second.sh_flags);
            std::println("   addr : 0x{:X}", section.second.sh_addr);
            std::println("   offset : 0x{:X}", section.second.sh_offset);
            std::println("   size : 0x{:X}", section.second.sh_size);
            std::println("   link : 0x{:X}", section.second.sh_link);
            std::println("   info : 0x{:X}", section.second.sh_info);
            std::println("   addralign : 0x{:X}", section.second.sh_addralign);
            std::println("   entsize : 0x{:X}", section.second.sh_entsize);
            std::println("====================");
        }
    }
}

void ElfParser::print_program_header()
{
    if (m_program_header.empty()) {
        std::print(stderr,
                   "Error (print_program_header): No program headers exists.");
    } else {
        for (int i = 0; i < m_elf_header.e_phnum; i++) {
            std::println("Program Header: {}", i);
            std::println("====================");
            std::println("type : 0x{:X}", m_program_header[i].p_type);
            std::println("flags : 0x{:X}", m_program_header[i].p_flags);
            std::println("offset : 0x{:X}", m_program_header[i].p_offset);
            std::println("vaddr : 0x{:X}", m_program_header[i].p_vaddr);
            std::println("paddr : 0x{:X}", m_program_header[i].p_paddr);
            std::println("filez : 0x{:X}", m_program_header[i].p_filesz);
            std::println("memz : 0x{:X}", m_program_header[i].p_memsz);
            std::println("align : 0x{:X}", m_program_header[i].p_align);
            std::println("====================");
        }
    }
}

std::variant<uint32_t, uint64_t> ElfParser::get_section_addr(
  std::string_view p_section)
{
    std::variant<uint32_t, u_int64_t> section_addr;
    try {
        section_addr = m_section_header.at(p_section).sh_addr;
    } catch (const std::out_of_range& e) {
        std::println(stderr,
                     "Error (get_section_addr) {}: Section Does Not Exist: {}",
                     e.what(),
                     p_section);
    }
    return section_addr;
}

std::variant<uint32_t, uint64_t> ElfParser::get_section_offset(
  std::string_view p_section)
{
    std::variant<uint32_t, u_int64_t> section_offset;
    try {
        section_offset = m_section_header.at(p_section).sh_offset;
    } catch (const std::out_of_range& e) {
        std::println(stderr,
                     "Error (get_section_offset): Section Does Not Exist: {}",
                     e.what());
    }
    return section_offset;
}

std::variant<uint32_t, uint64_t> ElfParser::get_section_size(
  std::string_view p_section)
{
    std::variant<uint32_t, u_int64_t> section_size;
    try {
        section_size = m_section_header.at(p_section).sh_size;
    } catch (const std::out_of_range& e) {
        std::println(stderr,
                     "Error (get_section_size): Section Does Not Exist: {}",
                     e.what());
    }
    return section_size;
}

std::vector<std::byte> ElfParser::get_section_data(std::string_view p_section)
{
    std::vector<std::byte> section_data;
    try {
        section_data = m_section_data.at(p_section);
    } catch (const std::out_of_range& e) {
        std::println(stderr,
                     "Error (get_section_data): Section Does Not Exist: {}",
                     e.what());
    }
    return section_data;
}
