#pragma once
#include <err.h>
#include <fcntl.h>
#include <libelf.h> 
#include <gelf.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>
#include <print>
#include <bitset>

using namespace std;

// typedef struct
// {
//     unsigned char	e_ident[EI_NIDENT];	/* Magic number and other info */
//     Elf64_Half	e_type;			/* Object file type */
//     Elf64_Half	e_machine;		/* Architecture */
//     Elf64_Word	e_version;		/* Object file version */
//     Elf64_Addr	e_entry;		/* Entry point virtual address */
//     Elf64_Off	e_phoff;		/* Program header table file offset */
//     Elf64_Off	e_shoff;		/* Section header table file offset */
//     Elf64_Word	e_flags;		/* Processor-specific flags */
//     Elf64_Half	e_ehsize;		/* ELF header size in bytes */
//     Elf64_Half	e_phentsize;		/* Program header table entry size */
//     Elf64_Half	e_phnum;		/* Program header table entry count */
//     Elf64_Half	e_shentsize;		/* Section header table entry size */
//     Elf64_Half	e_shnum;		/* Section header table entry count */
//     Elf64_Half	e_shstrndx;		/* Section header string table index */
// } Elf64_Ehdr;

class Elf_parser
{
    public: 
        Elf_parser(char **file_p) : file(file_p) {};
        void openElf ();
        void printEhdr();
        //getSection_header
        //getProgram_header
        //get_lsda
        //printElf
    private:
      int elfClass = 0;
      int fd;

      char **file;

      Elf* e; 

      GElf_Ehdr ehdr;
};

void Elf_parser::openElf()
{
    //Checks version of the elf
    if ( elf_version(EV_CURRENT) == EV_NONE ) 
    {
        errx(EX_SOFTWARE , "ELF library initialization failed : %s " , elf_errmsg ( -1));
    }
    //opens elf file
    if (( fd = open ( file[1] , O_RDONLY , 0)) < 0)
    {
        err (EX_NOINPUT , "open \%s\" failed " , file[1]);
    }
    //reads elf file
    if (( e = elf_begin ( fd , ELF_C_READ , NULL )) == NULL )
    {
        errx (EX_SOFTWARE , "elf_begin () failed : %s . " , elf_errmsg ( -1)); 
    }
    //Checks if file is an elf object
    if(elf_kind (e) != ELF_K_ELF)
    {
        errx(EX_DATAERR, "\"%s\" is not an ELF object. ", file[1]);
    }
    //prints bit version of elf object
    println("{} {}-bit ELF object\n", file[1], elfClass == ELFCLASS32 ? 32 : 64);
}

void Elf_parser::printEhdr()
{
    if((gelf_getehdr(e, &ehdr)) == NULL)
    {
        errx(EX_SOFTWARE, "getehdr() failed: %s.", elf_errmsg(-1));
    }
    println("ELF Header");
    println("====================");
    print("ident: ");
    for(int i = 0; i < 16; i++)
    {
        print("0x{:X}, ", ehdr.e_ident[i]);
    }
    println("");
    println("type: 0x{:X}", ehdr.e_type);
    println("machine: 0x{:X}", ehdr.e_machine);
    println("version: 0x{:X}", ehdr.e_version);
    println("entry: 0x{:X}", ehdr.e_entry);
    println("phoff: 0x{:X}", ehdr.e_phoff);
    println("shoff: 0x{:X}", ehdr.e_shoff);
    println("flags: 0x{:X}", ehdr.e_flags);
    println("ehsize: 0x{:X}", ehdr.e_ehsize);
    println("phentsize: 0x{:X}", ehdr.e_phentsize);
    println("shentsize: 0x{:X}", ehdr.e_shentsize);
    println("shnum: 0x{:X}", ehdr.e_shnum);
    println("phnum: 0x{:X}", ehdr.e_phnum);
}