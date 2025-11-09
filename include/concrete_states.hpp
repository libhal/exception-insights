#pragma once
#include <print>
#include <optional>
#include <memory>
#include "./state.hpp"

class UserInputState : public State {
    public: 
        void enter(StateContext& context) override;
        void handle(StateContext& context) override;
        std::optional<std::unique_ptr<State>> exit(StateContext& context) override;
};

class ElfParserState : public State {
    public: 
        void enter(StateContext& context) override;
        void handle(StateContext& context) override;
        std::optional<std::unique_ptr<State>> exit(StateContext& context) override;
};

class CallgraphState : public State {
    public: 
        void enter(StateContext& context) override;
        void handle(StateContext& context) override;
        std::optional<std::unique_ptr<State>> exit(StateContext& context) override;
};

class AbiParserState : public State {
    public: 
        void enter(StateContext& context) override;
        void handle(StateContext& context) override;
        std::optional<std::unique_ptr<State>> exit(StateContext& context) override;
};

class ValidatorState : public State {
    public: 
        void enter(StateContext& context) override;
        void handle(StateContext& context) override;
        std::optional<std::unique_ptr<State>> exit(StateContext& context) override;
};

class OutputState : public State {
    public: 
        void enter(StateContext& context) override;
        void handle(StateContext& context) override;
        std::optional<std::unique_ptr<State>> exit(StateContext& context) override;
};

class ErrorState : public State {
    public: 
        void enter(StateContext& context) override;
        void handle(StateContext& context) override;
        std::optional<std::unique_ptr<State>> exit(StateContext& context) override;
};