/**
 * @file abi_parser.cpp
 * @author Kenny Tran
 * @brief ABI parser implementation file
 * @version 0.1
 * @date 2025-09-11
 *
 * @copyright Copyright (c) 2025
 *
 **/

#include "abi_parse.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>

void AbiParser::check(size_t n) const
{
    if (index + n > data.size()) {
        throw std::runtime_error("LSDA read out of bounds");
    }
}

// reads 1 byte and then it goes forward with cursor
uint8_t AbiParser::read8()
{
    check(1);
    return data[index++];
}

// reads 2 bytes and makes a 16 bit integer

uint16_t AbiParser::read16()
{
    check(2);
    uint16_t v = 0;
    for (int i = 0; i < 2; ++i) {
        v |= static_cast<uint16_t>(data[index++]) << (i * 8);
    }
    return v;
}
// reads 4 bytes and makes a 32 bit integer
uint32_t AbiParser::read32()
{
    check(4);
    uint32_t v = 0;
    for (int i = 0; i < 4; ++i) {
        v |= static_cast<uint32_t>(data[index++]) << (i * 8);
    }
    return v;
}
// reads 8 bytes and makes a 64 bit integer
uint64_t AbiParser::read64()
{
    check(8);
    uint64_t value = 0;
    for (int i = 0; i < 8; ++i) {
        value |= (static_cast<uint64_t>(data[index++]) << (i * 8));
    }
    return value;
}

void AbiParser::build_scopes()
{
    scopes.clear();

    for (const auto& cs : call_sites) {
        if (cs.landing_pad == 0) {
            continue;
        }

        Scope s;
        s.start = cs.start;
        s.end = cs.start + cs.length;

        // LSDA: action is a byte offset into the action table (0 = none)
        if (cs.action == 0) {
            scopes.push_back(std::move(s));
            continue;
        }

        const int64_t action_offset = cs.action;

        // Map offset -> first action index
        int64_t action_index = -1;
        for (size_t i = 0; i < actions.size(); ++i) {
            if (actions[i].entry_offset == action_offset) {
                action_index = static_cast<int64_t>(i);
                break;
            }
        }

        if (action_index < 0) {
            throw std::runtime_error("invalid LSDA: call-site action offset "
                                     "not found in action table");
        }

        // Follow action chain using resolved next_index
        while (action_index >= 0) {
            const Action& a = actions[static_cast<size_t>(action_index)];

            ScopeHandler h;
            h.type_index = a.type;
            h.landing_pad = cs.landing_pad;

            if (a.type == 0) {
                h.type = HandlerType::Cleanup;
            } else if (a.type > 0) {
                h.type = HandlerType::Catch;
            } else {  // a.type < 0
                h.type = HandlerType::Filter;
            }

            s.handlers.push_back(h);
            action_index
              = a.next_index;  // <- offset logic now hidden in next_index
        }

        scopes.push_back(std::move(s));
    }
}

AbiParser::AbiParser(const std::vector<std::byte>& lsda_data)
{
    data.reserve(lsda_data.size());
    for (std::byte b : lsda_data) {
        data.push_back(static_cast<uint8_t>(b));
    }
    parse();
}

AbiParser::AbiParser(const std::vector<uint8_t>& lsda_data)
{
    data = lsda_data;      // copy into owned storage
    parse();
}

uint64_t AbiParser::read_uleb128(const std::vector<uint8_t>& buf, size_t& i)
{
    uint64_t result = 0;
    int shift = 0;
    while (i < buf.size()) {
        uint8_t byte = buf[i++];
        result |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) {
            break;
        }
        shift += 7;
        if (shift >= 64) {
            throw std::runtime_error("ULEB128 overflow");
        }
    }
    return result;
}

int64_t AbiParser::read_sleb128(const std::vector<uint8_t>& buf, size_t& i)
{
    int64_t result = 0;
    int shift = 0;
    uint8_t byte = 0;
    while (i < buf.size()) {
        byte = buf[i++];
        result |= static_cast<int64_t>(byte & 0x7F) << shift;
        shift += 7;
        if ((byte & 0x80) == 0) {
            break;
        }
        if (shift >= 64) {
            throw std::runtime_error("SLEB128 overflow");
        }
    }
    if ((shift < 64) && (byte & 0x40)) {
        result |= -(1LL << shift);
    }
    return result;
}

// read encode value
uint64_t AbiParser::r_encode(uint8_t encoding, uint64_t pcrel)
{
    if (encoding == 0xFF) {
        return 0;  // omitted
    }

    const bool indirect = (encoding & 0x80) != 0;  // indirection flag
    const uint8_t app = (encoding & 0x70);         // application
    const uint8_t form = (encoding & 0x0F);        // representation

    uint64_t value = 0;

    switch (form) {
        case 0x00:  // absptr
            value = read64();
            break;
        case 0x01:  // uleb128
            value = read_uleb128(data, index);
            break;
        case 0x02:  // udata2
            value = read16();
            break;
        case 0x03:  // udata4
            value = read32();
            break;
        case 0x04:  // udata8
            value = read64();
            break;
        case 0x09:  // sleb128
            value = static_cast<uint64_t>(read_sleb128(data, index));
            break;
        case 0x0B:  // sdata4
            value = static_cast<int32_t>(read32());
            break;
        case 0x0C:  // sdata8
            value = static_cast<int64_t>(read64());
            break;
        default:
            throw std::runtime_error("unsupported DW_EH_PE form");
    }

    if (indirect) {
        throw std::runtime_error("indirect doesn't support raw LSDA");
    }

    if (app == 0x10) {
        value += pcrel;
    }

    return value;
}

// parse
void AbiParser::parse()
{
    call_sites.clear();
    actions.clear();
    type_table.clear();
    scopes.clear();
    index = 0;

    // print status
    std::cout << "Parsing LSDA...\n\n";

    // header
    uint8_t start_enc = 0xFF;
    uint8_t tt_enc = 0xFF;
    uint64_t tt_off = 0;
    parse_header(start_enc, tt_enc, tt_off);

    const size_t tt_start
      = (tt_enc != 0xFF) ? (index + static_cast<size_t>(tt_off)) : data.size();

    // callsite table descriptor
    const uint8_t call_enc = read8();
    const uint64_t call_site_len = read_uleb128(data, index);
    const size_t call_site_end = index + static_cast<size_t>(call_site_len);
    if (call_site_end > data.size()) {
        throw std::runtime_error("call site table went over LSDA size");
    }

    parse_call_sites(call_enc, call_site_len);

    // index is at beginning of action table
    const size_t actions_start = index;
    // remaining bytes = action table
    const size_t actions_limit = std::min(tt_start, data.size());
    if (index > actions_limit) {
        throw std::runtime_error("actions start past type table");
    }
    parse_actions_tail(actions_start, actions_limit);

    // parse type table if given
    if (tt_enc != 0xFF) {
        if (tt_start > data.size()) {
            throw std::runtime_error("type table start beyond LSDA size");
        }

        // Type table starts at tt_start and runs to end of LSDA
        index = tt_start;
        while (index < data.size()) {
            // r_encode advances index according to tt_enc
            uint64_t type_addr = r_encode(tt_enc, 0);
            type_table.push_back(type_addr);
        }
    }

    // Build high-level scopes from call sites + actions
    build_scopes();
}

std::optional<uint64_t> AbiParser::resolve_type(int64_t type_index) const
{
    // Cleanup and filter entries have no concrete type address
    if (type_index <= 0) {
        return std::nullopt;
    }

    // Itanium: 1-based, reversed indexing into the type table
    const size_t n = type_table.size();
    const size_t idx = static_cast<size_t>(type_index);

    if (idx == 0 || idx > n) {
        return std::nullopt;
    }

    const size_t pos = n - idx;  // 1 -> last, 2 -> second-to-last, etc.
    return type_table[pos];
}

// parse LSDA header
void AbiParser::parse_header(uint8_t& start_enc,
                             uint8_t& tt_enc,
                             uint64_t& tt_off)
{
    start_enc = read8();
    if (start_enc != 0xFF) {
        const uint64_t pcrel_base = static_cast<uint64_t>(index);
        std::ignore = r_encode(start_enc, pcrel_base);  // reads and ignores
    }

    tt_enc = read8();
    if (tt_enc != 0xFF) {
        tt_off = read_uleb128(data, index);
    } else {
        tt_off = 0;
    }
}

// parse callsite table
void AbiParser::parse_call_sites(uint8_t call_enc, uint64_t table_len)
{
    size_t end = index + static_cast<size_t>(table_len);
    while (index < end) {
        CallSite cs{};
        cs.start = r_encode(call_enc, 0);
        cs.length = r_encode(call_enc, 0);
        cs.landing_pad = r_encode(call_enc, 0);
        cs.action = read_sleb128(data, index);
        call_sites.push_back(cs);
    }
    if (index != end) {
        throw std::runtime_error("call site parsing did not end at limit");
    }
}

// parse action table end
void AbiParser::parse_actions_tail(size_t table_start, size_t limit_end)
{
    while (index < limit_end) {
        Action a{};
        a.entry_offset = static_cast<int64_t>(index - table_start);
        a.type = read_sleb128(data, index);
        if (index > limit_end) {
            throw std::runtime_error("action parsing went over limit");
        }
        if (index == limit_end) {
            throw std::runtime_error(
              "malformed action table odd sleb128 count");
        }
        a.next_offset = read_sleb128(data, index);
        if (index > limit_end) {
            throw std::runtime_error("action parsing went over limit");
        }
        a.next_index = -1;
        actions.push_back(a);
    }

    // Second pass: resolve next_offset -> next_index
    for (size_t i = 0; i < actions.size(); ++i) {
        Action& a = actions[i];

        if (a.next_offset == 0) {
            a.next_index = -1;  // no next action in chain
            continue;
        }

        const int64_t target_offset = a.entry_offset + a.next_offset;

        int64_t found = -1;
        for (size_t j = 0; j < actions.size(); ++j) {
            if (actions[j].entry_offset == target_offset) {
                found = static_cast<int64_t>(j);
                break;
            }
        }

        if (found < 0) {
            // truncate chain when target is outside known entries
            // techdebt: handle cross LSDA/shared-tail action chains when we
            // parse full .gcc_except_table sections instead of single LSDAs.
            std::cerr << "[AbiParser] warning: next_offset from entry_offset="
                    << a.entry_offset << " points to unknown target_offset="
                    << target_offset << " – truncating action chain\n";
            a.next_index = -1;
            continue;
        }
        a.next_index = found;
    }
}

// printers
// NOTE: TEMPORARILY ADDED FILENAME PARAMETER FOR DEBUGGING
void AbiParser::print_call_sites(const std::string& filename) const
{
    std::ofstream out(filename);
    if (!out) {
        throw std::runtime_error("cannot open output file" + filename);
    }
    out << "\n[Call Site Table]\n";
    for (const auto& cs : call_sites) {
        out << "Start: 0x" << std::hex << cs.start << ", Length: 0x"
            << cs.length << ", LandingPad: 0x" << cs.landing_pad
            << ", Action: " << cs.action << "\n";
    }
}

void AbiParser::print_actions(const std::string& filename) const
{
    std::ofstream out(filename, std::ios::app);
    if (!out) {
        throw std::runtime_error("cannot open output file" + filename);
    }

    out << "\n[Action Table]\n";
    for (const auto& act : actions) {
        out << "EntryOffset: " << act.entry_offset << ", Type: " << act.type
            << ", NextOffset: " << act.next_offset
            << ", NextIndex: " << act.next_index << "\n";
    }
}
