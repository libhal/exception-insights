#include "abi_parse.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

// keep in mind of pathing!!!
int main(int argc, char** argv)
{
    // temporarily reading LSDA file before merging with main
    const char* path = (argc >= 2 ? argv[1] : "LSDA/lsda");
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        path = "LSDA/lsda";
        file.open(path, std::ios::binary);
    }
    if (!file) {
        std::cerr << "cannot open LSDA file\n";
        return 1;
    }

    std::vector<uint8_t> lsda_data((std::istreambuf_iterator<char>(file)),
                                   std::istreambuf_iterator<char>());
    GccParser parser(lsda_data);

    try {
        const auto& call_sites = parser.get_call_sites();
        const auto& actions = parser.get_actions();
        const auto& scopes = parser.get_scopes();

        std::cout << "[Call Sites] count=" << call_sites.size() << "\n";
        std::cout << "[Actions]    count=" << actions.size() << "\n";
        std::cout << "[Scopes]     count=" << scopes.size() << "\n";
        // parser.print_call_sites("LSDA/lsda_output.txt"); // temporary file
        // output parser.print_actions("LSDA/lsda_output.txt"); // temporary
        // file output

    } catch (const std::exception& e) {
        std::cerr << "parsing error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}