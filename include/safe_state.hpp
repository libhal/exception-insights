#pragma once
#include "./safe_state.hpp"

class Safe;

class SafeState {
    public: 
        virtual void enter(Safe* safe) = 0;
        virtual void toggle(Safe* safe) = 0;
        virtual void exit(Safe* safe) = 0;
        virtual ~SafeState() {};
};