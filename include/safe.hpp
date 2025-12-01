#pragma once
#include <expected>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "elf_parser.hpp"
#include "gcc_parse.hpp"

/**
 * @enum safe_error
 * @brief Error codes for safe operations.
 */
enum class safe_error : uint8_t
{
    INVALID_ARG_AMOUNT,  //!< Wrong argument amount
    INVALID_FLAG,        //!< Wrong flag
    FILE_NOT_FOUND       //!< File doe not exist
};

/**
 * @brief Error type variant of all error code types.
 */
typedef std::variant<safe_error, elf_parser_error> error_type;

/**
 * @struct symbol_s
 * @brief Structure representing an ELF symbol table entry.
 */
struct error_log_s
{
    error_type e_type;     //!< Error type of the error log
    std::string e_string;  //!< Error string message
};

/**
 * @class Safe
 * @brief Control Flow of all the operations of SAFE.
 *
 * This class provided the control flow of SAFE and handle when components
 * will run and holds the input and output data for each component
 *
 */
class Safe
{
  public:
    /**
     * @brief Construct a new Safe object
     *
     * @param argc Amount of arguments passed
     * @param argv C-string array of arguements
     * @throws error_log_s (Throws are mainly used to catch error logs in
     *         unit tests)
     */
    Safe(int argc, char* argv[]);

    /**
     * @brief Destroy the Safe object
     *
     */
    ~Safe() = default;

  private:
    std::string m_file_name;            //!< Name of file
    std::optional<std::string> m_flag;  //!< Flag to determine output option

    std::span<symbol_s> m_sym;      //!< ELF symbol table
    std::vector<std::byte> m_lsda;  //!< LSDA section
    std::vector<std::byte> m_text;  //!< .text section

    std::optional<error_log_s>
      m_last_error;  //!< Last error log before failure (Will be used in the
                     //!< future for logging)

    /**
     * @brief Get the args object
     *
     * @param argc Amount of arguments passed
     * @param argv C-string array of arguements
     * @return std::expected<void, error_log_s> Stores the file name and flag
     * (if exist) into m_file_name and m_flag. If function fails, return an
     * error log with error_type being a safe_error.
     */
    std::expected<void, error_log_s> get_args(int argc, char* argv[]);

    /**
     * @brief Get the elf object
     *
     * @return std::expected<void, error_log_s>  Parses out the targeted ELF
     * file and parses out .gcc_except_table(lsda), symbol table, and .text
     * into m_lsda, m_sym, and m_text.
     */
    std::expected<void, error_log_s> get_elf();
};