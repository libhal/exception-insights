#pragma once
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
#include <vector>
class ElfParser
{
  public:
    ElfParser(std::string_view p_file_name)
      : m_file(p_file_name)
    {
        if (elf_version(EV_CURRENT) == EV_NONE) {
            std::println(
              stderr, "ELF library initialization failed : {}", elf_errmsg(-1));
        }

        if ((m_fd = open(m_file.c_str(), O_RDONLY, 0)) < 0) {
            std::println(stderr, "\{} failed to open", elf_errmsg(-1));
            fileOpened = false;
        } else {
            fileOpened = true;
        }

        if ((m_elf = elf_begin(m_fd, ELF_C_READ, NULL)) == NULL) {
            std::println(
              stderr, "Elf file failed to load : {}.", elf_errmsg(-1));
            fileLoaded = false;
        } else {
            fileLoaded = true;
        }

        if (fileLoaded && fileOpened) {
            if (elf_kind(m_elf) != ELF_K_ELF) {
                std::println(stderr, "\{} is not an ELF object. ", m_file);
            }
            std::println("{} {}-bit ELF object\n",
                         m_file,
                         m_elf_class == ELFCLASS32 ? 32 : 64);
            loadElfHeader();
            loadSectionHeader();
            loadProgramHeader();
        }
    };

    ~ElfParser()
    {
        close(m_fd);
        std::println("ELF file closed");
    }

    void printElfHeader();
    void printSectionHeader();
    void printProgramHeader();

    void getSection(std::string_view const& section);

  private:
    int m_elf_class;
    int m_fd;
    std::string m_file;

    Elf* m_elf;
    GElf_Ehdr m_elfHeader;
    std::vector<GElf_Phdr> m_programHeader;
    std::unordered_map<std::string, GElf_Shdr> m_sectionHeader;

    bool elfHeaderLoaded;
    bool fileOpened;
    bool fileLoaded;

    void loadElfHeader();
    void loadSectionHeader();
    void loadProgramHeader();
};
