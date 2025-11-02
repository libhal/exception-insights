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
#include <print>
#include <tuple>

#include "gcc_parse.hpp"

int main(int argc, char** argv)
{
    std::ignore = argc;
    std::ignore = argv;
    std::println("Hello C++: {}", __cplusplus);

    safe::parse_gcc_wpa(
      "testing_programs/build/demo_class.wpa.081i.whole-program");
    return 0;
}