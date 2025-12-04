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

#include <expected>
#include <gelf.h>
#include <libelf.h>
#include <print>
#include <span>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

/**
 * @enum elf_parser_error
 * @brief Error codes for ELF parsing operations.
 */
enum class elf_parser_error : uint8_t
{
    UNLOADED_ELF_HEADER,  //!< ELF header has not been loaded
    SECTION_NOT_FOUND,    //!< Requested section does not exist
    PROGRAM_NOT_FOUND,    //!< Requested program header does not exist
    SYMBOL_NOT_FOUND,     //!< Requested symbol does not exist
    EMPTY_SECTION,        //!< Section map is empty
    EMPTY_PROGRAM,        //!< Program header vector is empty
    EMPTY_SYMBOL          //!< Symbol table vector is empty
};

/**
 * @struct symbol_s
 * @brief Structure representing an ELF symbol table entry.
 */
struct symbol_s
{
    std::string_view name;  //!< Symbol name from string table
    std::variant<uint32_t, uint64_t>
      value;             //!< Symbol value (address or constant)
    unsigned char info;  //!< Symbol type and binding attributes
};

/**
 * @struct section_s
 * @brief Structure representing an ELF section with its header and data.
 */
struct section_s
{
    GElf_Shdr header;             //!< Section header information
    std::vector<std::byte> data;  //!< Section data bytes
};

/**
 * @class ElfParser
 * @brief Parser for ELF (Executable and Linkable Format) files.
 *
 * This class provides functionality to parse and extract information from ELF
 * binary files, supporting both 32-bit and 64-bit architectures. It uses the
 * libelf library to read ELF headers, sections, program headers, and symbol
 * tables. All sections, program headers, and symbols are loaded upon
 * construction for efficient querying.
 */
class ElfParser
{
  public:
    /**
     * @brief Constructs an ElfParser and opens the specified ELF file.
     *
     * Opens the ELF file, validates it, and automatically loads all headers:
     * - Initializes the libelf library with elf_version(EV_CURRENT)
     * - Opens the file in read-only mode using open()
     * - Validates the file is a proper ELF object with elf_kind()
     * - Loads ELF header via m_load_elf_header()
     * - Loads all section headers and data via m_load_section_header()
     * - Loads all program headers via m_load_program_header()
     * - Loads symbol table via m_load_symbol_table()
     *
     * Prints ELF class information (32-bit or 64-bit) to stdout upon successful
     * initialization.
     *
     * @param p_file_name Path to the ELF file to be parsed.
     * @throws std::runtime_error Caught internally, prints error and calls
     * exit(EXIT_FAILURE) for:
     *         - ELF library initialization failures
     *         - elf_begin() failures
     *         - Non-ELF object files
     * @throws std::system_error Caught internally, prints error and calls
     * exit(EXIT_FAILURE) for:
     *         - File open failures
     */
    ElfParser(std::string_view p_file_name);

    /**
     * @brief Destroys the ElfParser and releases associated resources.
     *
     * Closes the ELF descriptor using elf_end() and the file descriptor using
     * close(). Prints "ELF file closed." confirmation message to stdout.
     */
    ~ElfParser();

    /**
     * @brief Retrieves the ELF file header.
     *
     * Returns the parsed ELF header containing:
     * - Magic number (ident bytes)
     * - File type, machine architecture, and version
     * - Entry point address
     * - Program header and section header offsets
     * - Flags and various header sizes
     *
     * @return std::expected<GElf_Ehdr, elf_parser_error> The ELF header on
     * success, or UNLOADED_ELF_HEADER error if the header was not loaded during
     * construction.
     */
    std::expected<GElf_Ehdr, elf_parser_error> get_elf_header();

    /**
     * @brief Retrieves a specific section by name.
     *
     * Searches the m_sections map for the named section and returns its
     * structure including:
     * - Section header with type, flags, alignment, addresses, and sizes
     * - Section data as a vector of bytes (empty for SHT_NOBITS sections)
     *
     * @param p_section Name of the section to retrieve (e.g., ".text", ".data",
     * ".symtab").
     * @return std::expected<section_s, elf_parser_error> The section structure
     * on success, or EMPTY_SECTION if m_sections is empty, or SECTION_NOT_FOUND
     * if the named section does not exist.
     */
    std::expected<section_s, elf_parser_error> get_section(
      std::string_view p_section);

    /**
     * @brief Retrieves all program headers.
     *
     * Returns a span view of all program headers (segments) containing:
     * - Type (PT_LOAD, PT_DYNAMIC, etc.) and flags
     * - File offset
     * - Virtual and physical addresses
     * - File size and memory size
     * - Alignment
     *
     * @return std::expected<std::span<GElf_Phdr>, elf_parser_error> A span of
     * program headers on success, or EMPTY_PROGRAM if m_program_header vector
     * is empty.
     */
    std::expected<std::span<GElf_Phdr>, elf_parser_error> get_program_header();

    /**
     * @brief Retrieves the symbol table.
     *
     * Returns a span view of all symbols parsed from the .symtab section,
     * including:
     * - Symbol names (from .strtab string table)
     * - Symbol values (addresses or constants)
     * - Symbol type and binding information (st_info)
     *
     * @return std::expected<std::span<symbol_s>, elf_parser_error> A span of
     * symbols on success, or EMPTY_SYMBOL if m_symbol_table vector is empty.
     * @note Returns empty vector (EMPTY_SYMBOL) if .symtab or .strtab sections
     * are not present.
     */
    std::expected<std::span<symbol_s>, elf_parser_error> get_symbol_table();

  private:
    int m_elf_class;  //!< ELF class identifier (ELFCLASS32 or ELFCLASS64).
    int m_file;       //!< File descriptor for the opened ELF file.
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
     * @brief Map of section names to their structures.
     *
     * Provides O(1) lookup of sections by name. Keys are string_views
     * pointing to section names from the string table. Each value contains
     * both the section header and its data. Populated during construction
     * by m_load_section_header().
     */
    std::unordered_map<std::string_view, section_s> m_sections;

    /**
     * @brief Collection of parsed symbol table entries.
     *
     * Stores all symbols from the .symtab section with names resolved from
     * .strtab. Populated during construction by m_load_symbol_table().
     */
    std::vector<symbol_s> m_symbol_table;

    /**
     * @brief Flag indicating whether the ELF header has been successfully
     * loaded.
     *
     * Set to true by m_load_elf_header() on success, false on failure.
     * Checked by m_load_section_header() and m_load_program_header() before
     * proceeding.
     */
    bool m_elf_header_loaded;

    /**
     * @brief Parses and loads the ELF file header.
     *
     * Calls gelf_getehdr() to read the ELF header from m_elf and stores it
     * in m_elf_header. Sets m_elf_header_loaded to true on success, false on
     * failure.
     *
     * @note Prints error message to stderr using elf_errmsg(-1) if
     * gelf_getehdr() fails, but does not throw an exception or exit.
     */
    void m_load_elf_header();

    /**
     * @brief Parses and loads all section headers and their data.
     *
     * Iterates through all sections using elf_nextscn(), extracting for each:
     * - Section header via gelf_getshdr()
     * - Section name from string table via elf_strptr() using e_shstrndx
     * - Section data via elf_getdata()
     *
     * Special handling:
     * - SHT_NOBITS sections store empty data vectors
     * - Failed sections print error to stderr but iteration continues
     *
     * Populates m_sections map with section_s structures containing both
     * headers and data.
     *
     * @note Requires m_elf_header_loaded to be true. Prints error to stderr and
     * returns early if header not loaded. Individual section failures print
     * errors but don't stop processing.
     */
    void m_load_section_header();

    /**
     * @brief Parses and loads all program headers.
     *
     * Iterates through all program headers using gelf_getphdr() based on
     * e_phnum from m_elf_header. Stores each successfully retrieved program
     * header in m_program_header vector.
     *
     * @note Requires m_elf_header_loaded to be true. Prints error to stderr and
     * returns early if header not loaded. Individual program header failures
     * print errors but don't stop processing remaining headers.
     */
    void m_load_program_header();

    /**
     * @brief Parses and loads the symbol table.
     *
     * Searches for .symtab and .strtab sections in m_sections. If both exist:
     * - Calculates symbol count using sh_size / sh_entsize from .symtab header
     * - Iterates through symbol entries, casting raw bytes to GElf_Sym
     * structures
     * - Resolves symbol names from .strtab using st_name offset
     * - Creates symbol_s structures with name, value (st_value), and info
     * (st_info)
     * - Empty string used for symbols with st_name == 0
     *
     * Populates m_symbol_table with all parsed symbols. Returns silently
     * without loading any symbols if either .symtab or .strtab is missing.
     *
     * @note Does not print errors if sections are missing, allowing ELF files
     * without symbol tables to parse successfully.
     */
    void m_load_symbol_table();
};