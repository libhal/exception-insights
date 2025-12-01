/**
 * @file abi_parser.hpp
 * @author Kenny Tran
 * @brief ABI parser header file
 * @version 0.1
 * @date 2025-09-11
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

// single entry in LSDA call site
struct CallSite
{
    uint64_t start;        // start of protected range
    uint64_t length;       // length of protected range
    uint64_t landing_pad;  // landing pad address
    int64_t action;        // offset into to action table
};

// action within call site table
struct Action
{
    int64_t type;          // index type
    int64_t next_offset;   // next action in list
    int64_t entry_offset;  // offset to next action
    int64_t next_index;    // index of next action
};
enum class HandlerType
{
    Catch,    // when type_index > 0
    Cleanup,  // when type_index == 0
    Filter    // when type_index < 0
};

struct ScopeHandler
{
    HandlerType type;
    int64_t type_index;    // unampped LSDA integer
    uint64_t landing_pad;  // landing pad address from callsite
};

struct Scope
{
    uint64_t start;
    uint64_t end;
    std::vector<ScopeHandler> handlers;
};

class GccParser
{
  public:
    explicit GccParser(const std::vector<std::byte>& lsda_data);
    explicit GccParser(const std::vector<uint8_t>& lsda_data);


    std::optional<uint64_t> resolve_type(int64_t type_index) const;
    void print_call_sites(const std::string& filename) const;
    void print_actions(const std::string& filename) const;

    std::vector<CallSite> call_sites;  // parsed call site records
    std::vector<Action> actions;       // parsed action records
    std::vector<uint64_t> type_table;  // parsed type table entries
    std::vector<Scope> get_scopes() const
    {
        return scopes;
    }

  private:
    void parse();
    void build_scopes();

    void check(size_t n)
      const;  // checks that n bytes remains in LSDA buffer before read

    std::vector<uint8_t> data;  // LSDA data taken in
    size_t index{ 0 };          // the parsing offset

    uint8_t read8();    // reads 1 byte
    uint16_t read16();  // reads 2 bytes
    uint32_t read32();  // reads 4 bytes
    uint64_t read64();  // reads 8 bytes

    std::vector<Scope> scopes;

    // decoders
    static uint64_t read_uleb128(const std::vector<uint8_t>& data,
                                 size_t& index);
    static int64_t read_sleb128(const std::vector<uint8_t>& data,
                                size_t& index);

    // encoded value reader
    uint64_t r_encode(uint8_t encoding,
                      uint64_t pcrel = 0);  // pcrel for DW_EH_PE_pcrel flag

    void parse_header(uint8_t& start_enc, uint8_t& tt_enc, uint64_t& tt_off);
    void parse_call_sites(uint8_t call_enc, uint64_t table_len);
    void parse_actions_tail(size_t table_start, size_t limit_end);
};
