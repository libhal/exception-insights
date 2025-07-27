/**
 * @file main.cpp
 * @author SAFE Group
 * @brief main file of SAFE
 * @version 0.1
 * @date 2025-07-17
 *
 * @copyright Copyright (c) 2025
 *
 */

#include <fstream>
#include <print>
#include <string_view>

#include "../include/elf_parser.hpp"
int main(int argc, char* argv[])
{
    const std::string_view lsda = ".gcc_except_table";

    if (argc < 2) {
        std::print(stderr, "usage : %s file - name {}", argv[0]);
    }

    ElfParser elf(argv[1]);

    uint64_t address = std::get<uint64_t>(elf.get_section_addr(lsda));
    uint64_t offset = std::get<uint64_t>(elf.get_section_offset(lsda));
    uint64_t size = std::get<uint64_t>(elf.get_section_size(lsda));
    std::println("Name: {}", lsda);
    std::println("Address: 0x{:X}", address);
    std::println("Offset: 0x{:X}", offset);
    std::println("Size: 0x{:X}", size);

    std::vector<std::byte> lsdaData = elf.get_section_data(std::string(lsda));

    std::ofstream outfile("test/binary/lsda", std::ios::out | std::ios::binary);
    outfile.write(reinterpret_cast<const char*>(lsdaData.data()),
                  lsdaData.size());

    return 0;
}
