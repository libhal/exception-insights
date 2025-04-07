#pragma once
#include <bitset>
#include <err.h>
#include <fcntl.h>
#include <gelf.h>
#include <libelf.h>
#include <print>
#include <stdlib.h>
#include <string>
#include <sysexits.h>
#include <unistd.h>

class ElfParser
{
  public:
    ElfParser(char** t_file_ptr)
      : m_file(t_file_ptr){};
    void openElf();
    void printEhdr();
    void printPhdr();
    void printShdr();
    //void printSym();
    void getSection(std::string section);
    void closeElf();

  private:
    int m_elf_class{ 0 };
    int m_fd, m_fbin;
    size_t m_n, m_shstrndx;

    char** m_file;

    char* m_name;
    Elf_Scn* m_scn;
    Elf_Data* m_data;

    Elf* m_e;
    GElf_Ehdr m_ehdr;
    GElf_Phdr m_phdr;
    GElf_Shdr m_shdr;
    //Gelf_Sym m_sym;
};

void ElfParser::openElf()
{
    if (elf_version(EV_CURRENT) == EV_NONE) {
        errx(EX_SOFTWARE,
             "ELF library initialization failed : %s ",
             elf_errmsg(-1));
    }
    if ((m_fd = open(m_file[1], O_RDONLY, 0)) < 0) {
        err(EX_NOINPUT, "open \%s\" failed ", m_file[1]);
    }
    if ((m_e = elf_begin(m_fd, ELF_C_READ, NULL)) == NULL) {
        errx(EX_SOFTWARE, "elf_begin () failed : %s . ", elf_errmsg(-1));
    }
    if (elf_kind(m_e) != ELF_K_ELF) {
        errx(EX_DATAERR, "\"%s\" is not an ELF object. ", m_file[1]);
    }
    std::println(
      "{} {}-bit ELF object\n", m_file[1], m_elf_class == ELFCLASS32 ? 32 : 64);
}

void ElfParser::printEhdr()
{
    if ((gelf_getehdr(m_e, &m_ehdr)) == NULL) {
        errx(EX_SOFTWARE, "getehdr() failed: %s.", elf_errmsg(-1));
    }
    std::println("ELF Header");
    std::println("====================");
    std::print("ident: ");
    for (int i = 0; i < 16; i++) {
        std::print("0x{:X}, ", m_ehdr.e_ident[i]);
    }
    std::println("");
    std::println("type: 0x{:X}", m_ehdr.e_type);
    std::println("machine: 0x{:X}", m_ehdr.e_machine);
    std::println("version: 0x{:X}", m_ehdr.e_version);
    std::println("entry: 0x{:X}", m_ehdr.e_entry);
    std::println("phoff: 0x{:X}", m_ehdr.e_phoff);
    std::println("shoff: 0x{:X}", m_ehdr.e_shoff);
    std::println("flags: 0x{:X}", m_ehdr.e_flags);
    std::println("ehsize: 0x{:X}", m_ehdr.e_ehsize);
    std::println("phentsize: 0x{:X}", m_ehdr.e_phentsize);
    std::println("shentsize: 0x{:X}", m_ehdr.e_shentsize);
    std::println("shnum: 0x{:X}", m_ehdr.e_shnum);
    std::println("phnum: 0x{:X}", m_ehdr.e_phnum);
}

void ElfParser::printPhdr()
{
    if (elf_getphdrnum(m_e, &m_n) != 0) {
        errx(EX_SOFTWARE, "getphdrnum() failed: %s.", elf_errmsg(-1));
    }

    std::println("Program Header: ({})", static_cast<int>(m_n));
    std::println("====================");

    for (int i = 0; i < static_cast<int>(m_n); i++) {
        if (gelf_getphdr(m_e, i, &m_phdr) != &m_phdr) {
            errx(EX_SOFTWARE, "getphdr() failed: %s.", elf_errmsg(-1));
        }

        std::println("Program Header: {}", i);
        std::println("type : 0x{:X}", m_phdr.p_type);
        std::println("flags : 0x{:X}", m_phdr.p_flags);
        std::println("offset : 0x{:X}", m_phdr.p_offset);
        std::println("vaddr : 0x{:X}", m_phdr.p_vaddr);
        std::println("paddr : 0x{:X}", m_phdr.p_paddr);
        std::println("filez : 0x{:X}", m_phdr.p_filesz);
        std::println("memz : 0x{:X}", m_phdr.p_memsz);
        std::println("align : 0x{:X}", m_phdr.p_align);
        std::println("====================");
    }
}

void ElfParser::printShdr()
{
    if (elf_getshdrstrndx(m_e, &m_shstrndx) != 0) {
        errx(EX_SOFTWARE, "getshdrstrndx() failed: %s.", elf_errmsg(-1));
    }

    m_scn = NULL;

    std::println("Section Header: ({})", static_cast<int>(m_shstrndx));
    std::println("====================");

    while ((m_scn = elf_nextscn(m_e, m_scn)) != NULL) {
        if (gelf_getshdr(m_scn, &m_shdr) != &m_shdr) {
            errx(EX_SOFTWARE, "getshdr() failed: %s.", elf_errmsg(-1));
        }

        if ((m_name = elf_strptr(m_e, m_shstrndx, m_shdr.sh_name)) == NULL) {
            errx(EX_SOFTWARE, "elf_strptr() failed: %s.", elf_errmsg(-1));
        }

        std::println(
          "Section {}: {}", static_cast<uintmax_t>(elf_ndxscn(m_scn)), m_name);
        std::println("   type : 0x{:X}", m_shdr.sh_type);
        std::println("   flags : 0x{:X}", m_shdr.sh_flags);
        std::println("   addr : 0x{:X}", m_shdr.sh_addr);
        std::println("   offset : 0x{:X}", m_shdr.sh_offset);
        std::println("   size : 0x{:X}", m_shdr.sh_size);
        std::println("   link : 0x{:X}", m_shdr.sh_link);
        std::println("   info : 0x{:X}", m_shdr.sh_info);
        std::println("   addralign : 0x{:X}", m_shdr.sh_addralign);
        std::println("   entsize : 0x{:X}", m_shdr.sh_entsize);
        std::println("====================");
    }
}

void ElfParser::getSection(std::string section)
{
    if (elf_getshdrstrndx(m_e, &m_shstrndx) != 0) {
        errx(EX_SOFTWARE, "getshdrstrndx() failed: %s.", elf_errmsg(-1));
    }

    m_scn = NULL;

    while ((m_scn = elf_nextscn(m_e, m_scn)) != NULL) {
        if (gelf_getshdr(m_scn, &m_shdr) != &m_shdr) {
            errx(EX_SOFTWARE, "getshdr() failed: %s.", elf_errmsg(-1));
        }

        if ((m_name = elf_strptr(m_e, m_shstrndx, m_shdr.sh_name)) == NULL) {
            errx(EX_SOFTWARE, "elf_strptr() failed: %s.", elf_errmsg(-1));
        }

        if (static_cast<std::string>(m_name) == section) {
            std::println("{} found", static_cast<std::string>(m_name));
            std::println(
              "Section {}: {}", static_cast<uintmax_t>(elf_ndxscn(m_scn)), m_name);
            std::println("====================");

            m_fbin = open("binary/lsda", O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);

            while ((m_data = elf_getdata(m_scn, m_data)) != NULL) {
                write(m_fbin, m_data->d_buf, m_data->d_size);
            }

            close(m_fbin);
            break;
        }
    }
}

void ElfParser::closeElf()
{
    close(m_fd);
    std::println("ELF file closed");
}