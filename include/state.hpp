#pragma once
#include "./state_context.hpp"

class StateContext;

class State {
    protected: 
        StateContext *context;
    public: 
        virtual void enter() = 0;
        virtual void handle() = 0;
        virtual void exit() = 0;

        void set_context(StateContext *context);

        virtual ~State() = 0;
};