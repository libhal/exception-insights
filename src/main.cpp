#include "../include/elf_parser.hpp"
#include <bitset>
#include <print>
#include <string>

int main(int argc, char** argv)
{
    string lsda = ".gcc_except_table";
    string if (argc != 2)
    {
        errx(EX_USAGE, "usage : %s file - name ", argv[0]);
    }

    Elf_parser elf(argv);
    elf.openElf();
    elf.getLsda(lsda);

    elf.closeElf();
    return 0;
    ;
}