#pragma once
#include "./state_machine.hpp"
#include "./state_context.hpp"

class State {
    public: 
        virtual void enter(StateContext& context) = 0;
        virtual void handle(StateContext& context) = 0;
        virtual State* exit(StateContext& context) = 0;

        virtual ~State() = default;
};