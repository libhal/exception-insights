#include <print>
#include <bitset>
#include <string>
#include "../include/elf_parser.hpp"

using namespace std;

int main ( int argc , char **argv )
{
    string lsda = ".gcc_except_table";
    if ( argc != 2)
    {
        errx(EX_USAGE , "usage : %s file - name " , argv [0]);
    }

    Elf_parser elf(argv);
    elf.openElf();
    // elf.printEhdr();
    // elf.printPhdr();
    //elf.printShdr();
    elf.getlsda(lsda);

    elf.closeElf();
    return 0;;
}