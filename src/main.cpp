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
#include "concrete_states.hpp"
#include "state.hpp"
#include "state_context.hpp"
#include "state_machine.hpp"

#include <print>
#include <tuple>

int main(int argc, char** argv)
{
    std::ignore = argc;
    std::ignore = argv;
    std::println("Hello C++: {}", __cplusplus);

    StateMachine sm;
    sm.run_state();

    return 0;
}
