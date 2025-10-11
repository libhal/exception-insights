#pragma once

// class or struct idk
class StateContext {
    public:
        void inc_data() {data++;}
        int get_data() {return data;}
    private:
        int data = 0;
};