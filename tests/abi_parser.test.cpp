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

std::string_view test_file = "../../testing_programs/build/simple";
ElfParser elf(test_file);
std::vector<std::byte> lsda = elf.get_section(".gcc_except_table").value().data;

    "basic_lsda_parse"_test = [lsda] {
        std::println("=======================================");
        std::println("Parsing LSDA: ");
        std::println("=======================================");

        LsdaParser parser(lsda);

        try {
            const std::vector<CallSite> call_sites = parser.get_call_sites();
            const std::vector<Action> actions = parser.get_actions();
            const std::vector<Scope> scopes = parser.get_scopes();
            const std::vector<uint64_t> type_table = parser.get_type_table();
            std::println("  Call Sites: {}", call_sites.size());
            std::println("  Action Tables: {}", actions.size());
            std::println("  Scopes: {}", scopes.size());
            std::println("  Type Table: {}", type_table.size());
            std::println("");

            expect(call_sites.size() > 0_u) << "no call sites parsed";
            expect(actions.size()    > 0_u) << "no actions parsed";
            expect(scopes.size()     > 0_u) << "no scopes parsed";

        } catch (const std::exception& e) {
            std::cerr << "parsing error: " << e.what() << "\n";
            expect(false) << "exception thrown while parsing LSDA";
        }
    };

    "LSDA Header"_test = [lsda] {
        LsdaParser parser(lsda);

        parser.print_header();
    };

    "Callsite"_test = [lsda] {
        LsdaParser parser(lsda);

        std::vector<CallSite> call_sites = parser.get_call_sites();

        std::println("Callsite Table tables:\n-------------------------\n");

        for(const auto& call_site : call_sites){
            std::println("Callsite {{ start: {}, length: {}, landing_pad: {:#04}, action: {}}}\n",
                    call_site.start, call_site.length, call_site.landing_pad, call_site.action);
        }
    };

    "Action Table"_test = [lsda] {
        LsdaParser parser(lsda);

        std::vector<Action> actions = parser.get_actions();

        std::println("Action tables:\n-------------------------\n");

        for(size_t i = 0; i < std::min(size_t(20), actions.size()); ++i){
            const auto& action = actions[i];
            std::println("Action {{ type: {}, next_offset: {}, entry_offset: {}, next_index: {}, next_field_offset: {} }}\n",
                    action.type, action.next_offset, action.entry_offset, 
                    action.next_index, action.next_field_offset);
        }
    };
};
