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
#include <iostream>
#include <iomanip>
#include <fstream>

void Abi_parser::check(size_t n) const 
{
    if (index + n > data.size()) {
        throw std::runtime_error("random stop at LSDA reading");
    }
}

// reads 1 byte and then it goes forward with cursor
uint8_t Abi_parser::read8() 
{
    check(1);
    return data[index++];
}

// reads 2 bytes and makes a 16 bit integer

uint16_t Abi_parser::read16() 
{
    check(2);
    uint16_t v = 0;
    for (int i = 0; i < 2; ++i)
        v |= static_cast<uint16_t>(data[index++]) << (i * 8);
    return v;
}
// reads 4 bytes and makes a 32 bit integer
uint32_t Abi_parser::read32() 
{
    check(4);
    uint32_t v = 0;
    for (int i = 0; i < 4; ++i)
        v |= static_cast<uint32_t>(data[index++]) << (i * 8);
    return v;
}
// reads 8 bytes and makes a 64 bit integer
uint64_t Abi_parser::read64() 
{
    check(8);
    uint64_t value = 0;
    for (int i = 0; i < 8; ++i) {
        value |= (static_cast<uint64_t>(data[index++]) << (i * 8));
    }
    return value;
}

Abi_parser::Abi_parser(const std::vector<uint8_t>& lsda_data)
    : data(lsda_data), index(0) {}

uint64_t Abi_parser::read_uleb128(const std::vector<uint8_t>& buf, size_t& i) 
{
    uint64_t result = 0;
    int shift = 0;
    while (i < buf.size()) {
        uint8_t byte = buf[i++];
        result |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
        if (shift >= 64) throw std::runtime_error("ULEB128 overflow");
    }
    return result;
}

int64_t Abi_parser::read_sleb128(const std::vector<uint8_t>& buf, size_t& i) 
{
    int64_t result = 0;
    int shift = 0;
    uint8_t byte = 0;
    while (i < buf.size()) {
        byte = buf[i++];
        result |= static_cast<int64_t>(byte & 0x7F) << shift;
        shift += 7;
        if ((byte & 0x80) == 0) break;
        if (shift >= 64) throw std::runtime_error("SLEB128 overflow");
    }
    if ((shift < 64) && (byte & 0x40)) {
        result |= -(1LL << shift);
    }
    return result;
}

// read encode value
uint64_t Abi_parser::r_encode(uint8_t encoding, uint64_t pcrel) 
{
    if (encoding == 0xFF) {
        return 0; // omitted
    }

    const bool indirect = (encoding & 0x80) != 0; // indirection flag
    const uint8_t app  = (encoding & 0x70);// application 
    const uint8_t form = (encoding & 0x0F); // representation

    uint64_t value = 0;

    switch (form) {
        case 0x00: // absptr 
            value = read64();
            break;
        case 0x01: // uleb128
            value = read_uleb128(data, index);
            break;
        case 0x02: // udata2
            value = read16();
            break;
        case 0x03: // udata4
            value = read32();
            break;
        case 0x04: // udata8
            value = read64();
            break;
        case 0x09: // sleb128
            value = static_cast<uint64_t>(read_sleb128(data, index));
            break;
        case 0x0B: // sdata4
            value = static_cast<int32_t>(read32());
            break;
        case 0x0C: // sdata8
            value = static_cast<int64_t>(read64());
            break;
        default:
            throw std::runtime_error("unsupported DW_EH_PE form");
    }

    if (indirect) {
    throw std::runtime_error("indirect doesn't support raw LSDA");
    }

    if (app == 0x10 ) {
        value += pcrel;
    }

    return value;
}

// parse 
void Abi_parser::parse() 
{
    call_sites.clear();
    actions.clear();
    index = 0;

    // print status
    std::cout << "Parsing LSDA...\n\n";

    // header
    uint8_t  start_enc = 0xFF;
    uint8_t  tt_enc    = 0xFF;
    uint64_t tt_off    = 0;
    parse_header(start_enc, tt_enc, tt_off);

    const size_t tt_start = 
        (tt_enc != 0xFF) ? (index + static_cast<size_t>(tt_off)) : data.size();

    // callsite table descriptor
    const uint8_t call_enc = read8();
    const uint64_t call_site_len = read_uleb128(data, index);
    const size_t call_site_end = index + static_cast<size_t>(call_site_len);
    if (call_site_end > data.size()) {
        throw std::runtime_error("call site table went over LSDA size");
    }

    parse_call_sites(call_enc, call_site_len);

    // remaining bytes = action table
    const size_t actions_limit = std::min(tt_start, data.size());
    if (index > actions_limit) throw std::runtime_error("actions start past type table");
    parse_actions_tail(actions_limit);
}

// parse LSDA header
void Abi_parser::parse_header(uint8_t& start_enc, uint8_t& tt_enc, uint64_t& tt_off) 
{
    start_enc = read8();
    if (start_enc != 0xFF) {
        const uint64_t pcrel_base = static_cast<uint64_t>(index);
        (void) r_encode(start_enc, pcrel_base); // reads and ignores
    }

    tt_enc    = read8();
    if (tt_enc != 0xFF) {
        tt_off = read_uleb128(data, index);
    } else {
        tt_off = 0;
    }
}

// parse callsite table
void Abi_parser::parse_call_sites(uint8_t call_enc, uint64_t table_len) 
{
    size_t end = index + static_cast<size_t>(table_len);
    while (index < end) {
        CallSite cs{};
        cs.start       = r_encode(call_enc, 0);
        cs.length      = r_encode(call_enc, 0);
        cs.landing_pad = r_encode(call_enc, 0);
        cs.action      = read_sleb128(data, index);
        call_sites.push_back(cs);
    }
    if (index != end) {
        throw std::runtime_error("call site parsing did not end at limit");
    }
}

// parse action table end
void Abi_parser::parse_actions_tail(size_t limit_end) {
    while (index < limit_end) {
        Action a{};
        a.type = read_sleb128(data, index);    
        if (index > limit_end)
            throw std::runtime_error("action parsing went over limit");
        if (index == limit_end)
        {
            throw std::runtime_error("malformed action table odd sleb128 count");
        }
        a.next = read_sleb128(data, index);        
        if (index > limit_end)
            throw std::runtime_error("action parsing went over limit");
        actions.push_back(a);
    }
}

// printers
// NOTE: TEMPORARILY ADDED FILENAME PARAMETER FOR DEBUGGING
void Abi_parser::print_call_sites(const std::string& filename) const 
{
    std::ofstream out(filename);
    if (!out) {
        throw std::runtime_error("cannot open output file" + filename);
    }
    out << "\n[Call Site Table]\n";
    for (const auto& cs : call_sites) {
            out << "Start: 0x" << std::hex << cs.start
                  << ", Length: 0x" << cs.length
                  << ", LandingPad: 0x" << cs.landing_pad
                  << ", Action: " << cs.action << "\n";
    }
}

void Abi_parser::print_actions(const std::string& filename) const 
{
    std::ofstream out(filename, std::ios::app);
    if (!out) {
        throw std::runtime_error("cannot open output file" + filename);
    }
    out << "\n[Action Table]\n";
    for (const auto& act : actions) {
            out << "Type: " << act.type
                  << ", Next: " << act.next << "\n";
    }
}