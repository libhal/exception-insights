#include <bitset>
#include <print>
#include <string>

#include "../include/elf_parser.hpp"
int main(int argc, char* argv[])
{
    //const std::string_view lsda = ".gcc_except_table";

    if (argc < 2)
    {
        std::print(stderr, "usage : %s file - name {}", argv[0]);
    }

    ElfParser elf(argv[1]);
    elf.printElfHeader();
    //elf.printSectionHeader();
    elf.printProgramHeader();

    return 0;
}