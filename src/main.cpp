#include "../include/abiparse.hpp"
#include <fstream>
#include <vector>
#include <iostream>
#include <string>
#include <filesystem>

// keep in mind of pathing!!!
int main(int argc, char** argv) 
{
    // temporarily reading LSDA file before merging with main
    const char* path = (argc >= 2 ? argv[1] : "LSDA/lsda");
    std::ifstream file(path, std::ios::binary);
    if (!file) 
    {
        path = "LSDA/lsda";
        file.open(path, std::ios::binary);
    }
    if (!file) 
    {
        std::cerr << "cannot open LSDA file\n";
        return 1;
    }

    std::vector<uint8_t> lsda_data(
        (std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    Abi_parser parser(lsda_data);

    try 
    {
        parser.parse();
        parser.print_call_sites("LSDA/lsda_output.txt"); // temporary file output
        parser.print_actions("LSDA/lsda_output.txt"); // temporary file output

    } catch (const std::exception& e) 
    {
        std::cerr << "parsing error: " << e.what() << "\n";
        return 1;
    }
    return 0;
    ;
}


