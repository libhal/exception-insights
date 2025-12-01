#include <elf.h>
#include <gelf.h>
#include <libelf.h>

#include <boost/ut.hpp>

#include <string_view>

#include "elf_parser.hpp"
#include "safe.hpp"

boost::ut::suite<"safe"> safe_tests = [] {
    using namespace boost::ut;
    
#if defined(__unix__) || defined(__APPLE__)
    std::system("cd ../../testing_programs/ && ./generate_and_build.sh");
#elif defined(_WIN32)
    std::system("cd ../../testing_programs/ && ./generate_and_build.ps1");
#endif

    "Invalid argument amount"_test = []() {
        char* arg_v[] = { const_cast<char*>("./safe"),
                          const_cast<char*>("-t"),
                          const_cast<char*>("./example_file.elf"),
                          const_cast<char*>("random"),
                          const_cast<char*>("even more random"),
                          const_cast<char*>("rawr"),
                          const_cast<char*>("67") };
        int arg_c = 7;
        try {
            Safe safe(arg_c, arg_v);
        } catch (error_log_s& el) {
            expect(std::get<safe_error>(el.e_type)
                   == safe_error::INVALID_ARG_AMOUNT)
              << "Expected safe_INVALID_ARG_AMOUNT" << el.e_string << "\n";
        }
    };

    "Invalid flag"_test = []() {
        char* arg_v[] = { const_cast<char*>("./safe"),
                          const_cast<char*>("bad_flag"),
                          const_cast<char*>("./testing_programs") };
        int arg_c = 3;
        try {
            Safe safe(arg_c, arg_v);
        } catch (error_log_s& el) {
            expect(std::get<safe_error>(el.e_type) == safe_error::INVALID_FLAG)
              << "Expected safe_INVALID_FLAG: " << el.e_string << "\n";
        }
    };

    "File not found (with flag)"_test = []() {
        char* arg_v_3arg[] = { const_cast<char*>("./safe"),
                               const_cast<char*>("-v"),
                               const_cast<char*>("./example_file.elf") };
        int arg_c_3arg = 3;

        try {
            Safe safe_3arg(arg_c_3arg, arg_v_3arg);
        } catch (error_log_s el) {
            expect(std::get<safe_error>(el.e_type)
                   == safe_error::FILE_NOT_FOUND)
              << "Expected safe_FILE_NOT_FOUND\n"
              << el.e_string << "\n";
        }
    };

    "File not found (without flag)"_test = []() {
        char* arg_v_2arg[] = { const_cast<char*>("./safe"),
                               const_cast<char*>("./example_file.elf") };
        int arg_c_2arg = 2;
        try {
            Safe safe_2arg(arg_c_2arg, arg_v_2arg);
        } catch (error_log_s& el) {
            expect(std::get<safe_error>(el.e_type)
                   == safe_error::FILE_NOT_FOUND)
              << "Expected safe_FILE_NOT_FOUND\n"
              << el.e_string << "\n";
        }
    };

    "Validating arguments (with flag)"_test = []() {
        char* arg_v_3arg[]
          = { const_cast<char*>("./safe"),
              const_cast<char*>("-v"),
              const_cast<char*>("../../testing_programs/build/demo_class") };
        int arg_c_3arg = 3;
        try {
            Safe safe_3arg(arg_c_3arg, arg_v_3arg);
            expect(true);
        } catch (error_log_s& el) {
            expect(false) << "Error should not exist:\n" << el.e_string << "\n";
        }
    };

    "Validating arguments (without flag)"_test = []() {
        char* arg_v_2arg[]
          = { const_cast<char*>("./safe"),
              const_cast<char*>("../../testing_programs/build/demo_class") };
        int arg_c_2arg = 2;
        try {
            Safe safe_2arg(arg_c_2arg, arg_v_2arg);
            expect(true);
        } catch (error_log_s& el) {
            expect(false) << "Error should not exist:\n" << el.e_string << "\n";
        }
    };
};
