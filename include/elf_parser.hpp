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

#include <fcntl.h>
#include <unistd.h>

#include <elf.h>
#include <gelf.h>
#include <libelf.h>

#include <print>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <variant>
#include <vector>
#include <system_error>
#include <span>
#include <expected>

enum class elf_parser_error{
  UNLOADED_ELF_HEADER,
  SECTION_NOT_FOUND,
  PROGRAM_NOT_FOUND,
  SYMBOL_NOT_FOUND,
  EMPTY_SECTION,
  EMPTY_PROGRAM,
  EMPTY_SYMBOL
};

typedef struct{
  std::string_view name;
  std::variant<uint32_t, uint64_t> value;
  unsigned char info;
}symbol_s;

typedef struct{
  GElf_Shdr header;
  std::vector<std::byte> data;
}section_s;


/** `
 * @class ElfParser
 * @brief Parser for ELF (Executable and Linkable Format) files.
 * 
 * This class provides functionality to parse and extract information from ELF
 * binary files, supporting both 32-bit and 64-bit architectures. It uses the
 * libelf library to read ELF headers, sections, and program headers. All sections
 * and program headers are loaded upon construction for efficient querying.
 */
class ElfParser
{
  public:
    /**
     * @brief Constructs an ElfParser and opens the specified ELF file.
     * 
     * Opens the ELF file, validates it, and automatically loads all headers:
     * - Initializes the libelf library
     * - Opens the file in read-only mode
     * - Validates the file is a proper ELF object
     * - Loads ELF header, section headers, and program headers into memory
     * 
     * @param p_file_name Path to the ELF file to be parsed.
     * @note This constructor calls exit(EXIT_FAILURE) on critical errors rather
     *       than throwing exceptions that propagate to the caller.
     */
    ElfParser(std::string_view p_file_name);

    /**
     * @brief Destroys the ElfParser and releases associated resources.
     * 
     * Closes the ELF file descriptor and prints a confirmation message to stdout.
     */
    ~ElfParser();

    /**
     * @brief Prints the ELF file header to standard output.
     * 
     * Displays all fields from the ELF header including:
     * - Magic number (ident bytes)
     * - File type, machine architecture, and version
     * - Entry point address
     * - Program header and section header offsets
     * - Flags and various header sizes
     * 
     * All numeric values are displayed in hexadecimal format.
     * 
     * @note Prints an error message to stderr if the ELF header was not loaded.
     */
    std::expected<GElf_Ehdr, elf_parser_error> get_elf_header();

    /**
     * @brief Prints all section headers to standard output.
     * 
     * Iterates through all loaded sections and displays detailed information:
     * - Section name
     * - Type, flags, and alignment
     * - Virtual address and file offset
     * - Size, link, info, and entry size
     * 
     * All numeric values are displayed in hexadecimal format.
     * 
     * @note Prints an error message to stderr if no section headers exist.
     */
    std::expected<section_s, elf_parser_error> get_section(std::string_view p_section);

    /**
     * @brief Prints all program headers to standard output.
     * 
     * Iterates through all loaded program headers (segments) and displays:
     * - Type and flags
     * - File offset
     * - Virtual and physical addresses
     * - File size and memory size
     * - Alignment
     * 
     * All numeric values are displayed in hexadecimal format.
     * 
     * @note Prints an error message to stderr if no program headers exist.
     */
    std::expected<std::span<GElf_Phdr>, elf_parser_error> get_program_header();

    std::expected<std::span<symbol_s>, elf_parser_error> get_symbol_table();

  private:
    int m_elf_class;          //!< ELF class identifier (ELFCLASS32 or ELFCLASS64).
    int m_file;               //!< File descriptor for the opened ELF file.
    std::string m_file_name;  //!< Path to the ELF file being analyzed.

    Elf* m_elf;              //!< Libelf handle for the ELF file.
    GElf_Ehdr m_elf_header;  //!< Parsed ELF file header structure.
    
    /**
     * @brief Collection of parsed program headers indexed by position.
     * 
     * Stores all program headers (PT_LOAD, PT_DYNAMIC, etc.) in order.
     * Populated during construction by m_load_program_header().
     */
    std::vector<GElf_Phdr> m_program_header;
    
    /**
     * @brief Map of section names to their header structures.
     * 
     * Provides O(1) lookup of section headers by name. Keys are string_views
     * pointing to section names from the string table. Populated during
     * construction by m_load_section_header().
     */
    std::unordered_map<std::string_view, section_s> m_sections;

    std::vector<symbol_s> m_symbol_table;

    /**
     * @brief Flag indicating whether the ELF header has been successfully loaded.
     * 
     * Set to true by m_load_elf_header() on success, false on failure.
     * Checked by m_load_section_header() and m_load_program_header() before proceeding.
     */
    bool m_elf_header_loaded;

    /**
     * @brief Parses and loads the ELF file header.
     * 
     * Calls gelf_getehdr() to read the ELF header from the file and stores it
     * in m_elf_header. Sets m_elf_header_loaded to true on success, false on failure.
     * 
     * @note Prints an error message to stderr using elf_errmsg() if loading fails,
     *       but does not throw an exception or exit.
     */
    void m_load_elf_header();

    /**
     * @brief Parses and loads all section headers and their data.
     * 
     * Iterates through all sections using elf_nextscn(), extracting:
     * - Section headers via gelf_getshdr()
     * - Section names from the string table via elf_strptr()
     * - Section data via elf_getdata()
     * 
     * Special handling for SHT_NOBITS sections (stores empty vectors).
     * Populates both m_section_header and m_section_data maps.
     * 
     * @note Requires m_elf_header_loaded to be true. Prints error messages to
     *       stderr for individual failures but continues processing remaining sections.
     */
    void m_load_section_header();

    /**
     * @brief Parses and loads all program headers.
     * 
     * Iterates through all program headers using gelf_getphdr() based on
     * e_phnum from the ELF header. Stores each program header in m_program_header.
     * 
     * @note Requires m_elf_header_loaded to be true. Prints error messages to
     *       stderr for individual failures but continues processing remaining headers.
     */
    void m_load_program_header();

    void m_load_symbol_table();
};