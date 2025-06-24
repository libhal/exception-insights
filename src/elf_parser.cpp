#include "../include/elf_parser.hpp"

void ElfParser::loadElfHeader()
{
    if ((gelf_getehdr(m_elf, &m_elfHeader)) == NULL) {
        std::print(stderr, "Elf header failed to load: {}.", elf_errmsg(-1));
        elfHeaderLoaded = false;
    } else {
        elfHeaderLoaded = true;
    }
}

void ElfParser::loadSectionHeader()
{
    if (!elfHeaderLoaded) {
        std::print(stderr, "Elf header was not previously loaded: {}.", elf_errmsg(-1));
    } else {
        char* sectionName;
        Elf_Scn* scn = nullptr;
        GElf_Shdr tempSectionHeader;
        while ((scn = elf_nextscn(m_elf, scn)) != NULL) {
            if (gelf_getshdr(scn, &tempSectionHeader) != &tempSectionHeader) {
                std::print(
                  stderr, "Unable to get section header: {}.", elf_errmsg(-1));
            }

            if ((sectionName = elf_strptr(m_elf,
                                          m_elfHeader.e_shstrndx,
                                          tempSectionHeader.sh_name)) == NULL) {
                std::print(
                  stderr, "Unable to get section name: {}.", elf_errmsg(-1));
            }

            m_sectionHeader.emplace(sectionName, tempSectionHeader);
        }
    }
}

void ElfParser::loadProgramHeader()
{
    if (!elfHeaderLoaded) {
        std::print(
          stderr, "Elf header was not previously loaded: {}.", elf_errmsg(-1));
    } else {
        GElf_Phdr tempProgramHeader;
        for (int i = 0; i < m_elfHeader.e_phnum; i++) {
            if (gelf_getphdr(m_elf, i, &tempProgramHeader) !=
                &tempProgramHeader) {
                std::print(
                  stderr, "Unable to get program header: {}.", elf_errmsg(-1));
            }
            m_programHeader.emplace_back(tempProgramHeader);
        }
    }
}

void ElfParser::printElfHeader()
{
    if (!elfHeaderLoaded) {
        std::print(
          stderr, "Elf header was not previously loaded: {}.", elf_errmsg(-1));
    } else {
        std::println("ELF Header");
        std::println("====================");
        std::print("ident: ");
        for (int i = 0; i < 16; i++) {
            std::print("0x{:X}, ", m_elfHeader.e_ident[i]);
        }
        std::println("");
        std::println("type: 0x{:X}", m_elfHeader.e_type);
        std::println("machine: 0x{:X}", m_elfHeader.e_machine);
        std::println("version: 0x{:X}", m_elfHeader.e_version);
        std::println("entry: 0x{:X}", m_elfHeader.e_entry);
        std::println("phoff: 0x{:X}", m_elfHeader.e_phoff);
        std::println("shoff: 0x{:X}", m_elfHeader.e_shoff);
        std::println("flags: 0x{:X}", m_elfHeader.e_flags);
        std::println("ehsize: 0x{:X}", m_elfHeader.e_ehsize);
        std::println("phentsize: 0x{:X}", m_elfHeader.e_phentsize);
        std::println("shentsize: 0x{:X}", m_elfHeader.e_shentsize);
        std::println("shnum: 0x{:X}", m_elfHeader.e_shnum);
        std::println("phnum: 0x{:X}", m_elfHeader.e_phnum);
    }
}

void ElfParser::printSectionHeader()
{
    if(m_sectionHeader.empty())
    {
        std::print(stderr, "No section headers exists.");
    } else {
        std::println("Section Header: ({})", m_elfHeader.e_shnum);
        std::println("====================");
        for(const auto& section : m_sectionHeader)
        {
            std::println(
            "Section: {}", section.first);
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

void ElfParser::printProgramHeader()
{
    if(m_programHeader.empty())
    {
        std::print(stderr, "No program headers exists.");
    } else {
        for(int i = 0; i < m_elfHeader.e_phnum; i++)
        {
        std::println("Program Header: {}", i);
        std::println("====================");
        std::println("type : 0x{:X}", m_programHeader[i].p_type);
        std::println("flags : 0x{:X}", m_programHeader[i].p_flags);
        std::println("offset : 0x{:X}", m_programHeader[i].p_offset);
        std::println("vaddr : 0x{:X}", m_programHeader[i].p_vaddr);
        std::println("paddr : 0x{:X}", m_programHeader[i].p_paddr);
        std::println("filez : 0x{:X}", m_programHeader[i].p_filesz);
        std::println("memz : 0x{:X}", m_programHeader[i].p_memsz);
        std::println("align : 0x{:X}", m_programHeader[i].p_align);
        std::println("====================");
        }
    }
}
