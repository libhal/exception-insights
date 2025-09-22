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

int main(int argc, char** argv)
{
    std::ignore = argc;
    std::ignore = argv;
    std::println("Hello C++: {}", __cplusplus);
#include "state.hpp"
#include "state_context.hpp"
#include "concrete_states.hpp"

void state_init() {
    UserInputState inital_state;
    StateContext *context = new StateContext(&inital_state);
    context->run_state();
    delete context;
}

int main()
{
    std::println("yeet: {}", __cplusplus);

    state_init();

    return 0;
}
