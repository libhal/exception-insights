#pragma once
#include <cstdint>
#include <vector>

// single entry in LSDA call site 
struct CallSite 
{
    uint32_t start;        // start of protected range
    uint32_t length;       // length of protected range
    uint32_t landing_pad;  // landing pad address
    int action;            // throw to action table
};

// action within call site table
struct Action 
{
    int type;              // index type
    int next;              // next action in list
};

class Abi_parser 
{
public:
    explicit Abi_parser(const std::vector<uint8_t>& lsda_data);
    void parse();

    void print_call_sites() const;
    void print_actions() const;

private:
    const std::vector<uint8_t>& data;  // raw LSDA byte data
    size_t index;                      // the parsing offset

    // this reads 1 byte from LSDA
    uint8_t read8();

    // reads 4 bytes
    uint32_t read32();

    std::vector<CallSite> call_sites;  // parsed call site records
    std::vector<Action> actions;       // parsed action records
};
