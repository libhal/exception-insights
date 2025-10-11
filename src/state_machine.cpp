#include "../include/state.hpp"
#include "../include/concrete_states.hpp"
#include "../include/state_machine.hpp"

#include <print>

StateMachine::StateMachine(){
    current_state = new UserInputState;
}

StateMachine::~StateMachine() {
    delete current_state;
}

State * StateMachine::get_current_state(){
    return current_state;
}

void StateMachine::run_state() {
    current_state->enter(context);
    current_state->handle(context);
    transition_state (current_state->exit(context));
}

void StateMachine::transition_state(State* new_state) {
    this->current_state = new_state;
}