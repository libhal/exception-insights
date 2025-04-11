#pragma once
#include <err.h>
#include <fcntl.h>
#include <libelf.h> 
#include <gelf.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>
#include <print>
#include <bitset>

using namespace std;

class Elf_parser
{
    public: 
        Elf_parser(char **file_p) : file(file_p) {};
        void openElf ();
        void printEhdr();
        void printPhdr();
        void printShdr();
        void getlsda(char *section);
        //printElf
    private:
        int elfClass = 0;
        int fd;
        size_t n, shstrndx;
        // sz;

        char **file;

        char * name;
        // *p , pc [4* sizeof ( char )];
        Elf_Scn * scn ;
        Elf_Data * data ;

        Elf* e; 
        GElf_Ehdr ehdr;
        GElf_Phdr phdr;
        GElf_Shdr shdr;
};

void Elf_parser::openElf()
{
    if ( elf_version(EV_CURRENT) == EV_NONE ) 
    {
        errx(EX_SOFTWARE , "ELF library initialization failed : %s " , elf_errmsg ( -1));
    }
    if (( fd = open ( file[1] , O_RDONLY , 0)) < 0)
    {
        err (EX_NOINPUT , "open \%s\" failed " , file[1]);
    }
    if (( e = elf_begin ( fd , ELF_C_READ , NULL )) == NULL )
    {
        errx (EX_SOFTWARE , "elf_begin () failed : %s . " , elf_errmsg ( -1)); 
    }
    if(elf_kind (e) != ELF_K_ELF)
    {
        errx(EX_DATAERR, "\"%s\" is not an ELF object. ", file[1]);
    }
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

void Elf_parser::printPhdr()
{
    if (elf_getphdrnum (e , &n) != 0) 
    {
        errx(EX_SOFTWARE, "getphdrnum() failed: %s.", elf_errmsg(-1));
    }

    println("Program Header: ({})", static_cast<int>(n));
    println("====================");

    for(int i = 0; i < static_cast<int>(n); i++)
    {
        if (gelf_getphdr (e , i, &phdr ) != &phdr )
        {
            errx(EX_SOFTWARE, "getphdr() failed: %s.", elf_errmsg(-1));
        }

        println("Program Header: {}", i);
        println("type : 0x{:X}", phdr.p_type);			
        println("flags : 0x{:X}", phdr.p_flags);		
        println("offset : 0x{:X}", phdr.p_offset);		
        println("vaddr : 0x{:X}", phdr.p_vaddr);		
        println("paddr : 0x{:X}", phdr.p_paddr);		
        println("filez : 0x{:X}", phdr.p_filesz);		
        println("memz : 0x{:X}", phdr.p_memsz);		
        println("align : 0x{:X}", phdr.p_align);
        println("====================");
    }		
}

void Elf_parser::printShdr()
{
    if (elf_getshdrstrndx (e , &shstrndx ) != 0) 
    {
        errx(EX_SOFTWARE, "getshdrstrndx() failed: %s.", elf_errmsg(-1));
    }

    scn = NULL;

    println("Section Header: ({})", static_cast<int>(shstrndx));
    println("====================");

    while((scn = elf_nextscn(e, scn)) != NULL)
    {
        if(gelf_getshdr(scn, &shdr ) != &shdr )
        {
            errx(EX_SOFTWARE, "getshdr() failed: %s.", elf_errmsg(-1));
        }

        if((name = elf_strptr(e, shstrndx, shdr.sh_name)) == NULL )
        {
            errx(EX_SOFTWARE, "elf_strptr() failed: %s.", elf_errmsg(-1));
        }

        println("Section {}: {}", static_cast<uintmax_t>(elf_ndxscn(scn)), name );		
        println("   type : 0x{:X}", shdr.sh_type);		
        println("   flags : 0x{:X}", shdr.sh_flags);		
        println("   addr : 0x{:X}", shdr.sh_addr);		
        println("   offset : 0x{:X}", shdr.sh_offset);		
        println("   size : 0x{:X}", shdr.sh_size);		
        println("   link : 0x{:X}", shdr.sh_link);		
        println("   info : 0x{:X}", shdr.sh_info);
        println("   addralign : 0x{:X}", shdr.sh_addralign);
        println("   entsize : 0x{:X}", shdr.sh_entsize);
        println("====================");
    }	
}

void Elf_parser::getlsda(char *section)
{
    if (elf_getshdrstrndx (e , &shstrndx ) != 0) 
    {
        errx(EX_SOFTWARE, "getshdrstrndx() failed: %s.", elf_errmsg(-1));
    }

    scn = NULL;

    while((scn = elf_nextscn(e, scn)) != NULL)
    {
        if(gelf_getshdr(scn, &shdr ) != &shdr )
        {
            errx(EX_SOFTWARE, "getshdr() failed: %s.", elf_errmsg(-1));
        }

        if((name = elf_strptr(e, shstrndx, shdr.sh_name)) == NULL )
        {
            errx(EX_SOFTWARE, "elf_strptr() failed: %s.", elf_errmsg(-1));
        }

        if(name == section)
        {
            break;
        }
    }
    
    println("Section {}: {}", static_cast<uintmax_t>(elf_ndxscn(scn)), name );	
    println("====================");
    data = null;
    n = 0;
    while (n < shdr.sh_size && (data = ef_getdata(scn, data)) != NULL)
    {
        //TODO (Michael): map the data bytes to LSDA format (See Khalil's slides at 143)
    }
}