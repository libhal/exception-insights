#include "../include/safe.hpp"
#include "../include/concrete_state.hpp"

Safe::Safe(){
    current_state = &UserInputState::get_instance();
}

void Safe::set_state(SafeState& new_state) {
    current_state->exit(this);
    current_state = &new_state;
    current_state->enter(this);
}

void Safe::toggle() {
    current_state->toggle(this);
}