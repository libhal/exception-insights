#pragma once
#include <print>
#include "./state.hpp"

class UserInputState : public State {
    public: 
        void enter() override;
        void handle() override;
        void exit() override;
};

class ElfParserState : public State {
    public: 
        void enter() override;
        void handle() override;
        void exit() override;
};

class CallgraphState : public State {
    public: 
        void enter() override;
        void handle() override;
        void exit() override;
};

class AbiParserState : public State {
    public: 
        void enter() override;
        void handle() override;
        void exit() override;
};

class ValidatorState : public State {
    public: 
        void enter() override;
        void handle() override;
        void exit() override;
};

class OutputState : public State {
    public: 
        void enter() override;
        void handle() override;
        void exit() override;
};

class ErrorState : public State {
    public: 
        void enter() override;
        void handle() override;
        void exit() override;
};