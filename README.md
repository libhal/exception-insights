# SAFE: Static Analyzer For Exceptions

Exceptions binary analysis tool (for Itanum ARM ABI)

## Code Structure

exception-insight
├── CMakeLists.txt
├── CMakeUserPresets.json
├── LICENSE
├── README.md
├── build
│ ├── Debug
│ │ ├── CMakeCache.txt
│ │ ├── CMakeFiles
│ │ ├── Makefile
│ │ ├── cmake_install.cmake
│ │ ├── compile_commands.json
│ │ ├── generators
│ │ ├── metadata
│ │ ├── safe
│ │ └── unit_test
│ └── logs
│ ├── RTTI_typeinfo.txt
│ └── function_binary.txt
├── compile_commands.json
├── conanfile.py
├── docs
│ ├── Doxyfile
│ ├── html
│ │ └── search
│ └── latex
│ └── Makefile
├── include
│ ├── abi_parse.hpp
│ ├── elf_parser.hpp
│ ├── gcc_parse.hpp
│ └── validator.hpp
├── src
│ ├── abi_parse.cpp
│ ├── elf_parser.cpp
│ ├── gcc_parse.cpp
│ ├── main.cpp
│ ├── throw.cpp
│ └── validator.cpp
├── testing_programs
│ ├── build
│ │ ├── demo_class
│ │ ├── elf_test
│ │ ├── multi_tu.whole-program
│ │ └── simple
│ ├── demo.cpp
│ ├── demo_class.cpp
│ ├── demo_two.cpp
│ ├── demo_two.h
│ ├── elf_test.cpp
│ ├── gcc_parse.py
│ ├── generate_and_build.ps1
│ ├── generate_and_build.sh
│ ├── shell.nix
│ ├── simple.cpp
│ ├── single_tu.cpp
│ └── test.c
└── tests
├── abi_parser.test.cpp
├── elf_parser.test.cpp
├── gcc_callgraph.test.cpp
├── main.test.cpp
├── testing.test.cpp
├── validator.test.cpp
└── validator_catch.test.cpp

# Build instructions

1. Build Command: `rm -r build && conan build . `
2. Run Command: `./build/Debug/safe <target ELF file> `
   - Run with test file command: `./build/Debug/safe testing_programs/build/simple `
