#pragma once
#include "./state.hpp"
#include "./state_context.hpp"

class State;

class StateMachine {
    public: 
        StateMachine();
        ~StateMachine();
        State* get_current_state();
        void run_state();
        void transition_state(State* new_state);
    private: 
        State *current_state;
        StateContext context;
};

