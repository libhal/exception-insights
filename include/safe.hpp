#pragma once
#include "./safe_state.hpp"

class SafeState;

class Safe {
    public: 
        Safe();
        inline SafeState* get_current_state() const {return current_state};
        void toggle();
        void set_state(SafeState& new_state);
    private: 
        SafeState* current_state;
};