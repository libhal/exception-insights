/**
 * @file elf_parser.hpp
 * @author Michael Chan
 * @brief ELF file parser header file
 * @version 0.1
 * @date 2025-07-17
 *
 * @copyright Copyright (c) 2025
 *
 */
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
    /**
     * @brief Constructor of the ElfParser object
     *
     * @param p_file_name
     */
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

    /**
     * @brief Destroy the Elf Parser object
     */
    ~ElfParser()
    {
        close(m_file);
        std::println("ELF file closed");
    }

    /**
     * @brief Prints the elf header stored in m_elfHeader
     */
    void printElfHeader();

    /**
     * @brief Prints out all section headers stored in m_sectionHeader.
     */
    void printSectionHeader();

    /**
     * @brief Prints out all program headers stored in m_programHeader.
     */
    void printProgramHeader();

    /**
     * @brief Returns the section address based on the given section name.The
     * address of the section depending on the ELF class of the ELF file.
     *
     * @param section
     * @return std::variant<uint32_t, uint64_t>
     */
    std::variant<uint32_t, uint64_t> getSectionAddr(std::string const& section);

    /**
     * @brief Returns the section offset based on the given section name. The
     * offset of the section depending on the ELF class of the ELF file.
     *
     * @param section
     * @return std::variant<uint32_t, uint64_t>
     */
    std::variant<uint32_t, uint64_t> getSectionOffset(
      std::string const& section);

    /**
     * @brief Returns the section size based on the given section name. The size
     * of the section depending on the ELF class of the ELF file
     *
     * @param section
     * @return std::variant<uint32_t, uint64_t>
     */
    std::variant<uint32_t, uint64_t> getSectionSize(std::string const& section);

    /**
     * @brief The section data based on the given section name.
     *
     * @param section Name of the section
     * @return std::vector<std::byte> The data of the given section. If the
     * section is the type NOBITS, the returning data will be null.
     */
    std::vector<std::byte> getSectionData(std::string const& section);

  private:
    int m_elf_class;  //!< Identifies the binary architecture of the ELF file.
    int m_file;       //!< The file meant to be analysed.
    std::string m_file_name;  //!< The file name.

    Elf* m_elf;             //!< ELF object from libelf.
    GElf_Ehdr m_elfHeader;  //!< ELF Header object.
    std::vector<GElf_Phdr>
      m_programHeader;  //!< Vector that stores ELF program header objects.
    std::unordered_map<std::string, GElf_Shdr>
      m_sectionHeader;  //!< Map that stores ELF section header objects.
    std::unordered_map<std::string, std::vector<std::byte>>
      m_sectionData;  //!< Map that stores the data from all ELF section.

    bool elfHeaderLoaded;  //!< Elf header object initialization flag
    bool fileOpened;       //!< Identifies if the file was able to be opened. 
    bool fileLoaded;       //!< Elf Object initialization flag
    /**
     * @brief Parses out the ELF file header and stores it into m_elfHeader.
     */
    void loadElfHeader();

    /**
     * @brief Parses out all section headers into m_sectionHeader and all
     * section. data into m_sectionData.
     */
    void loadSectionHeader();

    /**
     * @brief Parses out all headers and stores them into m_programHeader.
     */
    void loadProgramHeader();
};
