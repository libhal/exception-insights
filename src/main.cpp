#include <elf.h>

#include <bitset>
#include <print>
#include <string>
#include <fstream>

#include "../include/elf_parser.hpp"
int main(int argc, char* argv[])
{
    const std::string lsda = ".gcc_except_table";

    if (argc < 2) {
        std::print(stderr, "usage : %s file - name {}", argv[0]);
    }

    ElfParser elf(argv[1]);

    uint64_t address = std::get<uint64_t>(elf.getSectionAddr(lsda));
    uint64_t offset = std::get<uint64_t>(elf.getSectionOffset(lsda));
    uint64_t size = std::get<uint64_t>(elf.getSectionSize(lsda));
    std::println("Name: {}", lsda);
    std::println("Address: 0x{:X}", address);
    std::println("Offset: 0x{:X}", offset);
    std::println("Size: 0x{:X}", size);

    elf.printSectionHeader();

    std::vector<std::byte> test = elf.getSectionData(".note.gnu.build-id");

    std::vector<std::byte> lsdaData = elf.getSectionData(lsda);
        
    std::ofstream outfile("binary/lsda", std::ios::out | std::ios::binary);
    outfile.write(reinterpret_cast<const char*>(lsdaData.data()), lsdaData.size());

    return 0;
}