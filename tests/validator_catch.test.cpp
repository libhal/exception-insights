#include "validator_catch.hpp"
#include "abi_parse.hpp"
#include "elf_parser.hpp"
#include "validator.hpp"

#include <boost/ut.hpp>

#include <cstdlib>
#include <iostream>
#include <string_view>

using namespace std::string_view_literals;

boost::ut::suite validator_catch_test = [] {
    using namespace boost::ut;

#if defined(__unix__) || defined(__APPLE__)
    // small test program we parse LSDA from.
    std::system("cd ../../testing_programs/ && ./generate_and_build.sh");
#elif defined(_WIN32)
    std::system("cd ../../testing_programs/ && ./generate_and_build.ps1");
#endif

    "catch handler printer"_test = [] {
        std::string_view test_file = "../../testing_programs/build/simple";

        ElfParser elf(test_file);

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

        // section_s has std::vector<std::byte> data, feed that directly into
        // the LSDA parser.
        LsdaParser lsda(gcc_except_table.value().data);

        safe::CatchValidator validator{ lsda };

        const auto& records = validator.records();
        expect(records.size() > 0_u) << "no catch records parsed";

        for (const auto& rec : records) {
            expect(rec.range_begin <= rec.range_end) << "invalid handler range";
        }

        validator.print_records();
    };
    "throw-catch correlation"_test = [] {
        std::string_view test_file = "../../testing_programs/build/simple";

        ElfParser elf(test_file);

        // Symbol table and .text are for the throw-side Validator
        auto symtab = elf.get_symbol_table();
        expect(symtab.has_value()) << "no symbol table";

        auto text = elf.get_section(".text");
        expect(text.has_value()) << "missing .text section";

        // LSDA section for CatchValidator
        auto gcc_except_table = elf.get_section(".gcc_except_table");
        expect(gcc_except_table.has_value()) << "missing .gcc_except_table";

        // Build throw-side validator (your teammate's)
        safe::Validator throw_validator{ symtab.value(), text.value() };

        // Build LSDA-side validator (yours)
        LsdaParser lsda(gcc_except_table.value().data);
        safe::CatchValidator catch_validator{ lsda };

        // Function to inspect (mangled name); adjust if your test program
        // uses a different function name.
        constexpr auto func_name = "_Z3foov"sv;

        auto result
          = catch_validator.correlate_with_throws(throw_validator, func_name);

        expect(result.has_value()) << "correlation failed with error code";

        const auto& matches = result.value();
        expect(matches.size() > 0_u) << "no thrown types recorded for function";

        // At least one thrown type should have at least one matching handler.
        bool any_matched = false;
        for (const auto& m : matches) {
            if (!m.handlers.empty()) {
                any_matched = true;
                break;
            }
        }
        expect(any_matched) << "no LSDA catch handlers matched any thrown type";

        // Optional: print report for manual inspection
        catch_validator.print_throw_catch_report(throw_validator, func_name);
    };
};
