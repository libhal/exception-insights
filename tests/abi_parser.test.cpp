#include "abi_parse.hpp"

#include <boost/ut.hpp>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

boost::ut::suite<"Abi_Parser_Test"> abi_parser_test = [] {
    using namespace boost::ut;

    "basic_lsda_parse"_test = [] {
        // keep in mind of pathing!!!
        const char* path = "../../LSDA/lsda";

        std::ifstream file(path, std::ios::binary);
        if (!file) {
            // this is file path now
            std::cerr << "cannot open LSDA file at " << path << "\n";
            expect(false) << "LSDA file not found";
            return;
        }

        std::vector<std::uint8_t> lsda_data(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());

        LsdaParser parser(lsda_data);

        try {
            const auto& call_sites = parser.get_call_sites();
            const auto& actions    = parser.get_actions();
            const auto& scopes     = parser.get_scopes();

            expect(call_sites.size() > 0_u) << "no call sites parsed";
            expect(actions.size()    > 0_u) << "no actions parsed";
            expect(scopes.size()     > 0_u) << "no scopes parsed";

        } catch (const std::exception& e) {
            std::cerr << "parsing error: " << e.what() << "\n";
            expect(false) << "exception thrown while parsing LSDA";
        }
    };
};
