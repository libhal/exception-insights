#include "abi_parse.hpp"
#include "elf_parser.hpp"

#include <boost/ut.hpp>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

boost::ut::suite<"Abi_Parser_Test"> abi_parser_test = [] {
    using namespace boost::ut;
#if defined(__unix__) || defined(__APPLE__)
    // ensure the test binary exists (same pattern as other tests)
    std::system("cd ../../testing_programs/ && ./generate_and_build.sh");
#elif defined(_WIN32)
    std::system("cd ../../testing_programs/ && ./generate_and_build.ps1");
#endif

    "basic_lsda_parse"_test = [] {
        // keep in mind of pathing!!!
        const char* bin_path = "../../testing_programs/build/simple";


        ElfParser elf(bin_path);

        auto gcc_except_table = elf.get_section(".gcc_except_table");
        if (!gcc_except_table.has_value()) {
            std::cerr << "Failed to get .gcc_except_table section\nReason: ";
            if (gcc_except_table.error() == elf_parser_error::EMPTY_SECTION) {
                std::cerr << "Elf parser does not contain sections.\n";
            }
            if (gcc_except_table.error()
                == elf_parser_error::SECTION_NOT_FOUND) {
                std::cerr << "Section was not found.\n";
            }
            expect(false) << "missing .gcc_except_table";
            return;
        }

        LsdaParser parser(gcc_except_table.value().data);

        try {
            const auto& call_sites = parser.get_call_sites();
            const auto& actions    = parser.get_actions();
            const auto& scopes     = parser.get_scopes();

            expect(call_sites.size() > 0_u) << "no call sites parsed";
            expect(actions.size()    > 0_u) << "no actions parsed";
            expect(scopes.size()     > 0_u) << "no scopes parsed";

        } catch (const std::exception& e) {
            std::cerr << "parsing error: " << e.what() << "\n";
            expect(false) << "exception thrown while parsing LSDA";
        }
    };
};
