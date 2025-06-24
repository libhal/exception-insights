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
#include <fcntl.h>
#include <unistd.h>

#include <gelf.h>
#include <libelf.h>

#include <print>
#include <string_view>
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
    ElfParser(std::string_view p_file_name);

    /**
     * @brief Destroy the Elf Parser object
     */
    ~ElfParser();

    /**
     * @brief Prints the elf header
     */
    void print_elf_header();

    /**
     * @brief Prints out all section headers.
     */
    void print_section_header();

    /**
     * @brief Prints out all program headers.
     */
    void print_program_header();

    /**
     * @brief Returns the section address based on the given section name.The
     * address of the section depending on the ELF class of the ELF file.
     *
     * @param p_section
     * @return std::variant<uint32_t, uint64_t>
     */
    std::variant<uint32_t, uint64_t> get_section_addr(
      std::string_view p_section);

    /**
     * @brief Returns the section offset based on the given section name. The
     * offset of the section depending on the ELF class of the ELF file.
     *
     * @param p_section
     * @return std::variant<uint32_t, uint64_t>
     */
    std::variant<uint32_t, uint64_t> get_section_offset(
      std::string_view p_section);

    /**
     * @brief Returns the section size based on the given section name. The size
     * of the section depending on the ELF class of the ELF file
     *
     * @param p_section
     * @return std::variant<uint32_t, uint64_t>
     */
    std::variant<uint32_t, uint64_t> get_section_size(
      std::string_view p_section);

    /**
     * @brief The section data based on the given section name.
     *
     * @param p_section Name of the section
     * @return std::vector<std::byte> The data of the given section. If the
     * section is the type NOBITS, the returning data will be null.
     */
    std::vector<std::byte> get_section_data(std::string_view p_section);

  private:
    int m_elf_class;  //!< Identifies the binary architecture of the ELF file.
    int m_file;       //!< The file meant to be analysed.
    std::string m_file_name;  //!< The file name.

    Elf* m_elf;              //!< ELF object from libelf.
    GElf_Ehdr m_elf_header;  //!< ELF Header object.
    std::vector<GElf_Phdr>
      m_program_header;  //!< Vector that stores ELF program header objects.
    std::unordered_map<std::string_view, GElf_Shdr>
      m_section_header;  //!< Map that stores ELF section header objects.
    std::unordered_map<std::string_view, std::vector<std::byte>>
      m_section_data;  //!< Map that stores the data from all ELF section.

    bool m_elf_header_loaded;  //!< Elf header object initialization flag
<<<<<<< HEAD
=======
    bool m_file_opened;  //!< Identifies if the file was able to be opened.
    bool m_file_loaded;  //!< Elf Object initialization flag
>>>>>>> b975b74 (Reworked the parser to store the elf file headers in maps and vectors, move loading of the headers to the constructor, reformat to fit libhal formatting standard)

    /**
     * @brief Parses out the ELF file header and stores it into m_elf_header.
     */
    void m_load_elf_header();

    /**
     * @brief Parses out all section headers into m_section_header and all
     * section. data into m_sectionData.
     */
    void m_load_section_header();

    /**
     * @brief Parses out all headers and stores them into m_program_header.
     */
    void m_load_program_header();
};
