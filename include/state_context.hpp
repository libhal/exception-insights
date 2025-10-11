#pragma once

/**
 * @brief Content in this is temporary. Please change
 * them according to what the state desires. All data relating to 
 * SAFE will be stored here and shared between the states.
 *
 */
class StateContext {
    public:
        void inc_data() {data++;}
        int get_data() {return data;}
    private:
        int data = 0;
};