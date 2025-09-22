#pragma once
#include <cstdint>
#include <vector>

// single entry in LSDA call site 
struct CallSite 
{
    uint64_t start;        // start of protected range
    uint64_t length;       // length of protected range
    uint64_t landing_pad;  // landing pad address
    int64_t action;            // throw to action table
};

// action within call site table
struct Action 
{
    int64_t type;              // index type
    int64_t next;              // next action in list
};

class Abi_parser 
{
public:
    explicit Abi_parser(const std::vector<uint8_t>& lsda_data);
    void parse();

    void print_call_sites(const std::string& filename) const;
    void print_actions(const std::string& filename) const;

    std::vector<CallSite> call_sites;  // parsed call site records
    std::vector<Action> actions;       // parsed action records

private:
    void check(size_t n) const; // checks that n bytes remains in LSDA buffer before read 

    const std::vector<uint8_t>& data;  // raw LSDA byte data
    size_t index{0};                      // the parsing offset

    uint8_t read8(); // reads 1 byte
    uint16_t read16(); // reads 2 bytes
    uint32_t read32(); // reads 4 bytes
    uint64_t read64(); // reads 8 bytes

    // decoders
    static uint64_t read_uleb128(const std::vector<uint8_t>& data, size_t& index);
    static int64_t read_sleb128(const std::vector<uint8_t>& data, size_t& index);

    // encoded value reader
    uint64_t r_encode(uint8_t encoding, uint64_t pcrel = 0); // pcrel for DW_EH_PE_pcrel flag

    void parse_header(uint8_t& start_enc, uint8_t& tt_enc, uint64_t& tt_off);
    void parse_call_sites(uint8_t call_enc, uint64_t table_len);
    void parse_actions_tail(size_t limit_end);
};
