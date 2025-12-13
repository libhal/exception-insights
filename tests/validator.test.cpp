#include "validator.hpp"
#include "abi_parse.hpp"
#include "elf_parser.hpp"
#include "gcc_parse.hpp"

#include <boost/ut.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <print>
#include <string_view>
#include <vector>
#include <span>
#include <unordered_set> 

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
            auto sym = elf.get_symbol_table();
            if (!sym.has_value()) {
                expect(false) << "sym table fail\n";
            }
            auto text = elf.get_section(".text");
            if (!text.has_value()) {
                expect(false) << "text fail\n";
            }

            safe::Validator val(sym.value(), text.value());

            std::vector<symbol_s> throw_obj_foo
              = val.find_typeinfo("_Z3fooi").value();
            std::unordered_set<std::string_view> expected_typeinfo_foo
              = {   "typeinfo for std::invalid_argument",
                    "typeinfo for int",
                    "typeinfo for std::runtime_error",
                    "typeinfo for char const*",
                    "typeinfo for double" };

            std::println("Thrown objects from _Z3fooi");
            for (auto& obj : throw_obj_foo) {
                std::string mangled = val.demangle(obj.name.c_str()).value_or(obj.name);
                std::string type_name = mangled;
                std::println("  {}", type_name);
                expect(expected_typeinfo_foo.contains(type_name)) <<
                "Got: " << type_name << "\n";
            }

            std::vector<symbol_s> throw_obj_baa
              = val.find_typeinfo("_Z3baav").value();
            std::unordered_set<std::string_view> expected_typeinfo_baa
              = { "typeinfo for char const*" };

            std::println("Thrown objects from _Z3baav");
            for (auto& obj : throw_obj_baa) {
                std::string mangled = val.demangle(obj.name.c_str()).value_or(obj.name);
                std::string type_name = mangled;
                std::println("  {}", type_name);
                expect(expected_typeinfo_baa.contains(type_name)) <<
                "Got: " << type_name << "\n";
            }
        };

        "Exception correlation"_test = [test_file] {
            ElfParser elf(test_file);

            auto sym = elf.get_symbol_table();
            expect(sym.has_value()) << "sym table fail\n";

            auto text = elf.get_section(".text");
            expect(text.has_value()) << "text fail\n";

            auto gcc_except = elf.get_section(".gcc_except_table");
            expect(gcc_except.has_value()) << ".gcc_except_table fail\n";

            safe::Validator val(sym.value(), text.value());
            LsdaParser lsda(gcc_except->data);

            val.load_lsda(lsda);

            auto res = val.analyze_exceptions("_Z3fooi");
            expect(res.has_value()) << "analyze_exceptions failed\n";

            if (!res.has_value()) {
                std::println("analyze_exceptions error code = {}", static_cast<int>(res.error()));
                return;
            }

            bool any_caught = false;
            for (const auto& m : res.value()) {
                if (!m.handlers.empty()) { any_caught = true; break; }
            }
            expect(any_caught) << "No thrown types matched any catch handlers\n";
        };
    };
};
