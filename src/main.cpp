/**
 * @file main.cpp
 * @author SAFE Group
 * @brief main file of SAFE
 * @version 0.1
 * @date 2025-07-17
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "gcc_parse.hpp"
#include <print>
#include <stdexcept>

namespace rng = std::ranges;
namespace views = rng::views;

using string = std::string;
using std::operator""sv;

#include <fstream>
#include <print>
#include <string_view>

#include "../include/elf_parser.hpp"
int main(int argc, char* argv[])
{
    std::println("yeet: {}", __cplusplus);
    try {
        parse("./"
              "demo_class.wpa.081i.whole-program");
    } catch (std::runtime_error& e) {
        std::println("Error: {}", e.what());
        return 1;
    }

    return 0;
}
