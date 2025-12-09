#include "abi_parse.hpp"
#include "elf_parser.hpp"
#include "gcc_parse.hpp"

#include <boost/ut.hpp>

#include <fstream>
#include <vector>
#include <iostream>
#include <filesystem>
#include <string_view>

boost::ut::suite<"Validator_Test"> validator_test = [] {
    using namespace boost::ut;
#if defined(__unix__) || defined(__APPLE__)
    std::system("cd ../../testing_programs/ && ./generate_and_build.sh");
#elif defined(_WIN32)
    std::system("cd ../../testing_programs/ && ./generate_and_build.ps1");
#endif

    "Validator"_test = [] {
        std::string_view test_file = "../../testing_programs/build/simple";
    
        "Call graph checker"_test = [test_file] {
            ElfParser elf(test_file);

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

            GccParser abi(gcc_except_table.value().data);

            abi.print_call_sites(std::string(test_file));
            expect(true);
        };
    };
};
