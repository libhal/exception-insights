#include "../include/abiparse.hpp"
#include <fstream>
#include <vector>
#include <iostream>

// keep in mind of pathing!!!

int main() 
{
    std::ifstream file("LSDA/lsda.bin", std::ios::binary);
    if (!file) {
        std::cerr << "cant open lsda.bin\n";
        return 1;
    }

    std::vector<uint8_t> lsda_data((std::istreambuf_iterator<char>(file)), {});
    Abi_parser parser(lsda_data);

    parser.parse();
    parser.print_call_sites();
    parser.print_actions();

    return 0;
}


