#include "../include/elf_parser.hpp"
#include <bitset>
#include <print>
#include <string>

int main(int argc, char** argv)
{
    std::string lsda = ".gcc_except_table";
    std::string symtab = ".symtab";
    if (argc != 2)
    {
        errx(EX_USAGE, "usage : %s file - name ", argv[0]);
    }

    ElfParser elf(argv);
    elf.openElf();
    elf.printShdr();
    elf.getSection(lsda);

    elf.closeElf();
    return 0;
    ;
}