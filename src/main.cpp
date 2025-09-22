#include <print>

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