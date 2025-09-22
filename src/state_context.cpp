#include "../include/state.hpp"
#include "../include/concrete_state.hpp"de
#include "../include/state_context.hpp";

StateContext::StateContext(State *state){
    this->transition_state(state);
}

StateContext::~StateContext() {
    delete current_state
}

void StateContext::transition_state(State* new_state) {
    print("Transitioning\n");
    if(this->current_state != nullptr) {
        delete this->current_state;
    }
    this->current_state = new_state;
    this->current_state->set_context(this);
}

void StateContext::run_state() {
    self->current_state->enter();
    self->current_state->handle();
    self->current_state->exit();
}