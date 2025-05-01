#include "../include/abiparse.hpp"
#include <fstream>
#include <vector>
#include <iostream>

// keep in mind of pathing!!!

// more concentrated lsda
std::vector<uint8_t> con_lsda() {
    return {
        0x02,                               // 2 call sites
        0x00, 0x00, 0x00, 0x00,             // start 0
        0x10, 0x00, 0x00, 0x00,             // length 0x10
        0x20, 0x00, 0x00, 0x00,             // landing pad
        0x01,                               // action index 1

        0x10, 0x00, 0x00, 0x00,             // start 0x10
        0x10, 0x00, 0x00, 0x00,             // length 0x10
        0x40, 0x00, 0x00, 0x00,             // landing pad
        0x02,                               // action index 2

        0x02,                               // action count
        0x01, 0x02,                         // action 1: type=1, next=2
        0x02, 0x00                          // action 2: type=2, next=0
    };
}

int main() 
{
    // std::ifstream file("LSDA/lsda.bin", std::ios::binary);
    // if (!file) {
    //     std::cerr << "cant open lsda.bin\n";
    //     return 1;
    // }

    // std::vector<uint8_t> lsda_data((std::istreambuf_iterator<char>(file)), {});
    // Abi_parser parser(lsda_data);

    std::vector<uint8_t> lsda_data = con_lsda();
    Abi_parser parser(lsda_data);

    parser.parse();
    parser.print_call_sites();
    parser.print_actions();

    return 0;
}
