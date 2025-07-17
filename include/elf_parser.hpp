#pragma once
#include <elf.h>
#include <err.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

#include <gelf.h>
#include <libelf.h>

#include <print>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

class ElfParser
{
  public:
    ElfParser(std::string_view p_file_name)
      : m_file_name(p_file_name)
    {
        if (elf_version(EV_CURRENT) == EV_NONE) {
            std::println(
              stderr, "ELF library initialization failed : {}", elf_errmsg(-1));
        }

        if ((m_file = open(m_file_name.c_str(), O_RDONLY, 0)) < 0) {
            std::println(stderr, "\{} failed to open", elf_errmsg(-1));
            fileOpened = false;
        } else {
            fileOpened = true;
        }

        if ((m_elf = elf_begin(m_file, ELF_C_READ, NULL)) == NULL) {
            std::println(
              stderr, "Elf file failed to load : {}.", elf_errmsg(-1));
            fileLoaded = false;
        } else {
            fileLoaded = true;
        }

        if (fileLoaded && fileOpened) {
            if (elf_kind(m_elf) != ELF_K_ELF) {
                std::println(stderr, "\{} is not an ELF object. ", m_file_name);
            }
            std::println("{} {}-bit ELF object\n",
                         m_file_name,
                         m_elf_class == ELFCLASS32 ? 32 : 64);
            loadElfHeader();
            loadSectionHeader();
            loadProgramHeader();
        }
    };

    ~ElfParser()
    {
        close(m_file);
        std::println("ELF file closed");
    }

    void printElfHeader();
    void printSectionHeader();
    void printProgramHeader();

    std::variant<uint32_t, uint64_t> getSectionAddr(std::string const& section);
    std::variant<uint32_t, uint64_t> getSectionOffset(
      std::string const& section);
    std::variant<uint32_t, uint64_t> getSectionSize(std::string const& section);

    std::vector<std::byte> getSectionData(std::string const& section);

  private:
    int m_elf_class;
    int m_file;
    std::string m_file_name;

    Elf* m_elf;
    GElf_Ehdr m_elfHeader;
    std::vector<GElf_Phdr> m_programHeader;
    std::unordered_map<std::string, GElf_Shdr> m_sectionHeader;
    std::unordered_map<std::string, std::vector<std::byte>> m_sectionData;

    bool elfHeaderLoaded;
    bool fileOpened;
    bool fileLoaded;

    void loadElfHeader();
    void loadSectionHeader();
    void loadProgramHeader();
};
