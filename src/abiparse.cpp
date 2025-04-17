#include "abiparse.hpp"
#include <iostream>
#include <iomanip>

// reads 1 byte and then it goes forward with cursor
uint8_t Abi_parser::read8() 
{
    return data[index++];
}

// reads 4 bytes and makes a 32 bit integer
uint32_t Abi_parser::read32() 
{
    uint32_t value = 0;
    for (int i = 0; i < 4; ++i) {
        value |= (data[index++] << (i * 8));
    }
    return value;
}

Abi_parser::Abi_parser(const std::vector<uint8_t>& lsda_data)
    : data(lsda_data), index(0) {}

// this parses LSDA, then goes to call site, then action table
void Abi_parser::parse() 
{
    index = 0;
    std::cout << "Parsing LSDA...\n";

    size_t call_site_count = read8();
    for (size_t i = 0; i < call_site_count; ++i) {
        CallSite entry;
        entry.start = read32();
        entry.length = read32();
        entry.landing_pad = read32();
        entry.action = static_cast<int>(read8());
        call_sites.push_back(entry);
    }

    size_t action_count = read8();
    for (size_t i = 0; i < action_count; ++i) {
        Action act;
        act.type = static_cast<int>(read8());
        act.next = static_cast<int>(read8());
        actions.push_back(act);
    }
}

void Abi_parser::print_call_sites() const 
{
    std::cout << "\n[Call Site Table]\n";
    for (const auto& cs : call_sites) {
        std::cout << "Start: 0x" << std::hex << cs.start
                  << ", Length: 0x" << cs.length
                  << ", LandingPad: 0x" << cs.landing_pad
                  << ", Action: " << cs.action << "\n";
    }
}

void Abi_parser::print_actions() const 
{
    std::cout << "\n[Action Table]\n";
    for (const auto& act : actions) {
        std::cout << "Type: " << act.type
                  << ", Next: " << act.next << "\n";
    }
}