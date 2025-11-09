#pragma once
#include <optional>
#include <memory>
#include "./state.hpp"
#include "./state_context.hpp"

class State;

class StateMachine {
    public: 
        StateMachine();
        ~StateMachine();
        std::optional<std::reference_wrapper<State>> get_current_state();
        StateContext get_context();
        void run_state();
        void transition_state(std::optional<std::unique_ptr<State>> new_state);
    private: 
        std::optional<std::unique_ptr<State>> current_state;
        StateContext context;
};

