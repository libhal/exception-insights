#pragma once
#include "./safe_state.hpp"

class UserInputState : public SafeState {
    public: 
        void enter(Safe* safe) {}
        void toggle(Safe* safe);
        void exit(Safe* safe) {}
        static SafeState& get_instance();
    private:
        UserInputState() {};
        UserInputState(const UserInputState& other);
        UserInputState& operator = (const UserInputState& other); 
};

class ElfParserState : public SafeState {
    public: 
        void enter(Safe* safe) {}
        void toggle(Safe* safe);
        void exit(Safe* safe) {}
        static SafeState& get_instance();
    private:
        ElfParserState() {};
        ElfParserState(const ElfParserState& other);
        ElfParserState& operator = (const ElfParserState& other); 
};

class CallgraphState : public SafeState {
    public: 
        void enter(Safe* safe) {}
        void toggle(Safe* safe);
        void exit(Safe* safe) {}
        static SafeState& get_instance();
    private:
        CallgraphState() {};
        CallgraphState(const CallgraphState& other);
        CallgraphState& operator = (const CallgraphState& other); 
};

class AbiParserState : public SafeState {
    public: 
        void enter(Safe* safe) {}
        void toggle(Safe* safe);
        void exit(Safe* safe) {}
        static SafeState& get_instance();
    private:
        AbiParserState() {};
        AbiParserState(const AbiParserState& other);
        AbiParserState& operator = (const AbiParserState& other); 
};

class ValidatorState : public SafeState {
    public: 
        void enter(Safe* safe) {}
        void toggle(Safe* safe);
        void exit(Safe* safe) {}
        static SafeState& get_instance();
    private:
        ValidatorState() {};
        ValidatorState(const ValidatorState& other);
        ValidatorState& operator = (const ValidatorState& other); 
};

class OutputState : public SafeState {
    public: 
        void enter(Safe* safe) {}
        void toggle(Safe* safe);
        void exit(Safe* safe) {}
        static SafeState& get_instance();
    private:
        OutputState() {};
        OutputState(const OutputState& other);
        OutputState& operator = (const OutputState& other); 
};

class ErrorState : public SafeState {
    public: 
        void enter(Safe* safe) {}
        void toggle(Safe* safe);
        void exit(Safe* safe) {}
        static SafeState& get_instance();
    private:
        ErrorState() {};
        ErrorState(const ErrorState& other);
        ErrorState& operator = (const ErrorState& other); 
};