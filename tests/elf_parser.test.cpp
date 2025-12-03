#include <array>
#include <gelf.h>
#include <libelf.h>

#include <boost/ut.hpp>

#include <string_view>

#include "elf_parser.hpp"

boost::ut::suite<"Elf_Parser_Test"> elf_parser_test = [] {
    using namespace boost::ut;

    "Elf_Parser"_test = [] {
        std::string_view test_file = "../../testing_programs/elf_test";
        using namespace boost::ut;
        "Validate the Elf Header"_test = [test_file]() {
            ElfParser elf(test_file);
            std::array<unsigned char, 4> elf_magic_number
              = { 0x7F, 'E', 'L', 'F' };
            GElf_Ehdr ehdr = elf.get_elf_header().value();

            for (int i = 0; i < 4; i++) {
                expect(ehdr.e_ident[i] == elf_magic_number[i])
                  << "Expecting: " << elf_magic_number[i] << ". Got"
                  << ehdr.e_ident[i] << "\n";
            }
        };

        "Validate the Program Headers"_test = [test_file]() {
            ElfParser elf(test_file);
            auto ehdr = elf.get_elf_header().value();
            auto phdr = elf.get_program_header().value();

            expect(ehdr.e_phnum == phdr.size())
              << "Expect number of programs: " << ehdr.e_phnum << ". Got"
              << phdr.size() << "\n";
        };

        "Validate the Section Headers by finding .gcc_expect_table"_test =
          [test_file]() {
              ElfParser elf(test_file);
              std::string_view section_name = ".gcc_except_table";
              auto shdr = elf.get_section(section_name);

              expect(shdr.has_value()) << "Expect finding .gcc_except_table\n";
          };

        "Validate the Symbol Table"_test = [test_file]() {
            ElfParser elf(test_file);
            auto sym = elf.get_symbol_table();
            expect(sym.has_value()) << "Symbol Table must exist\n";

            auto symbols = sym.value();
            symbol_s main_symbol;
            bool found_main;

            for (const auto& symbol : symbols) {
                if (symbol.name == "main") {
                    found_main = true;
                    main_symbol = symbol;
                }
            }

            expect(found_main) << "main symbol does not exit";
            expect(main_symbol.name == "main")
              << "expected: main. Found" << main_symbol.name;
            expect(ELF64_ST_TYPE(main_symbol.info) == STT_FUNC)
              << "main should be a function";
            expect(ELF64_ST_BIND(main_symbol.info) == STB_GLOBAL)
              << "main should have global binding";
            uint64_t address = std::get<uint64_t>(main_symbol.value);
            expect(address > 0_u) << "main dddress should be non-zero";
        };
    };
};