#pragma once
#include "./state.hpp"

class State;

class StateContext {
    public: 
        StateContext(State *state);
        ~StateContext();
        State* get_current_state();
        void run_state();
        void transition_state(State* new_state);
    private: 
        State* current_state;
        int total_state;
};

