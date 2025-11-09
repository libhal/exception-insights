#include "../include/state.hpp"
#include "../include/concrete_states.hpp"
#include "../include/state_machine.hpp"

#include <print>

StateMachine::StateMachine(){
    current_state = std::make_unique<UserInputState>();
}

StateMachine::~StateMachine() = default;

std::optional<std::reference_wrapper<State>> StateMachine::get_current_state(){
    if(current_state){
        return std::ref(**current_state);
    }
    return std::nullopt;
}

StateContext StateMachine::get_context(){
    return context;
}

void StateMachine::run_state() {
    while(current_state != std::nullopt){
        current_state.value()->enter(context);
        current_state.value()->handle(context);
        transition_state (current_state.value()->exit(context));
    }
}

void StateMachine::transition_state(std::optional<std::unique_ptr<State>> new_state) {
    this->current_state = std::move(new_state);
}