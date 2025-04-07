#include <print>
#include <bitset>
#include "../include/elf_parser.hpp"

using namespace std;

//todo: label the outputs
//todo: read section and program headers
//todo: read out sections and headers
//todo: find and extract the lsda at gcc_except_table

int main ( int argc , char **argv )
{
    //Checks if number or args are valid
    if ( argc != 2)
    {
        errx(EX_USAGE , "usage : %s file - name " , argv [0]);
    }

    Elf_parser elf(argv);
    elf.openElf();
    elf.printEhdr();
    exit (EX_OK);
}