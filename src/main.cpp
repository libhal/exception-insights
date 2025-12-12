/**
 * @file main.cpp
 * @author SAFE Group
 * @brief main file of SAFE
 * @version 0.1
 * @date 2025-07-17
 *
 * @copyright Copyright (c) 2025
 *
 */
#include <expected>
#include <filesystem>
#include <print>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>
#include <unordered_set>
#include <iostream>
#include <string>

#include "abi_parse.hpp"
#include "elf_parser.hpp"
#include "validator_catch.hpp"

/**
 * @enum main_error
 * @brief Error codes for main expected functions.
 */
enum class main_error : uint8_t
{
    INVALID_ARG_AMOUNT,  //!< Wrong argument amount
    INVALID_FLAG,        //!< Wrong flag
    FILE_NOT_FOUND       //!< File doe not exist
};

/**
 * @brief holds the parsed arguements passed to the program. It can either be a
 * file_name and a flag or just the file name.
 *
 */
struct arg_value_s
{
    std::string_view file_name;
    std::optional<std::string_view> flag;
};

/**
 * @brief takes argv and argc, parses argv, and determines whether the arguments
 * are valid or not. Returns arg_value_s if successfull or a main_error enum if
 * failed.
 *
 * @param argc
 * @param argv
 * @return std::expected<arg_value_s, main_error>
 */
std::expected<arg_value_s, main_error> validate_args(int argc, char* argv[])
{
    if (argc == 3) {
        std::string flag(argv[1]);
        std::string file_name(argv[2]);
        if (flag != "-v") {
            std::print("Invalid Flag\n");
            return std::unexpected(main_error::INVALID_FLAG);
        }
        if (!std::filesystem::exists(file_name)) {
            std::print("File not found.\nFile: {}\n", file_name);
            return std::unexpected(main_error::FILE_NOT_FOUND);
        }
        return arg_value_s{ file_name, flag };
    } else if (argc == 2) {
        std::string file_name(argv[1]);
        if (!std::filesystem::exists(file_name)) {
            std::print("File not found.\nFile: {}\n", file_name);
            return std::unexpected(main_error::FILE_NOT_FOUND);
        }
        return arg_value_s{ file_name, std::nullopt };
    } else {
        std::print("Invalid argument amount\n");
        return std::unexpected(main_error::INVALID_ARG_AMOUNT);
    }
}

int main(int argc, char* argv[])
{
    auto args = validate_args(argc, argv);
    if (!args.has_value()) {
        exit(EXIT_FAILURE);
    }

    ElfParser elf(args.value().file_name);

    auto gcc_except_table = elf.get_section(".gcc_except_table");
    if (!gcc_except_table.has_value()) {
        std::print("Failed to get .gcc_except_table section\nReason: ");
        if (gcc_except_table.error() == elf_parser_error::EMPTY_SECTION) {
            std::print("Elf parser does not contain sections.\n");
        }
        if (gcc_except_table.error() == elf_parser_error::SECTION_NOT_FOUND) {
            std::print("Section was not found.\n");
        }
        exit(EXIT_FAILURE);
    }

    LsdaParser abi(gcc_except_table.value().data);

    return 0;
}