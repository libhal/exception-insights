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
#include "validator.hpp"

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
    std::string file_name;
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
        return EXIT_FAILURE;
    }

    ElfParser elf(args->file_name);

    auto sym = elf.get_symbol_table();
    if (!sym.has_value()) {
        std::print("Failed to get symbol table\n");
        return EXIT_FAILURE;
    }

    auto text = elf.get_section(".text");
    if (!text.has_value()) {
        std::print("Failed to get .text section\n");
        return EXIT_FAILURE;
    }

    safe::Validator val(sym.value(), text.value());

    auto gcc_except_table = elf.get_section(".gcc_except_table");
    if (!gcc_except_table.has_value()) {
        std::print("Failed to get .gcc_except_table section\nReason: ");
        if (gcc_except_table.error() == elf_parser_error::EMPTY_SECTION) {
            std::print("Elf parser does not contain sections.\n");
        }
        if (gcc_except_table.error() == elf_parser_error::SECTION_NOT_FOUND) {
            std::print("Section was not found.\n");
        }
        return EXIT_FAILURE;
    }

    LsdaParser lsda(gcc_except_table->data);

    // Load LSDA catch table into Validator
    val.load_lsda(lsda);

    auto res = val.analyze_exceptions("_Z3fooi");
    if (!res.has_value()) {
        std::print("analyze_exceptions failed\n");
        return EXIT_FAILURE;
    }

    for (const auto& f : val.find_thrown_functions()) {
        std::println("throws: {}",
            val.demangle(f.name.c_str()).value_or(f.name));
    }

    // we print it but we canremove this
    for (const auto& m : res.value()) {
        auto dn = val.demangle(m.thrown.name.c_str()).value_or(m.thrown.name);
        std::print("Thrown: {}\n", dn);
        for (auto* h : m.handlers) {
            std::print("  caught by {} type_index={}\n", h->scope_id, h->type_index);
        }
    }

    return 0;
}