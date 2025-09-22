#include "../include/concrete_state.hpp"
#include "../include/state.hpp"
#include "../include/state_context.hpp"

//User Input
void UserInputState::enter() {
    print("Enter User Input State\n");
}

void UserInputState::handle() {
    print("Handle User Input State\n");
}

void UserInputState::exit() {
    print("Exit User Input State\n-------------\n");
    this->contect->transition_state(new ElfParserState);
}

//Elf Parser
void ElfParserState::enter() {
    print("Enter Elf Parser State\n");
}

void ElfParserState::handle() {
    print("Handle Elf Parser State\n");
}

void ElfParserState::exit() {
    print("Exit Elf Parser State\n-------------\n");
    this->context->transition_state(new CallgraphState);
}

//Callgraph
void CallgraphState::enter() {
    print("Enter Callgraph State\n");
}

void CallgraphState::handle() {
    print("Handle Callgraph State\n");
}

void CallgraphState::exit() {
    print("Exit Callgraph State\n-------------\n");
    this->context->transition_state(new AbiParserState);
}

//Abi Parser
void AbiParserState::enter() {
    print("Enter ABI Parser State\n");
}

void AbiParserState::handle() {
    print("Handle ABI Parser State\n");
}

void AbiParserState::exit() {
    print("Exit ABI Parser State\n-------------\n");
    this->context->transition_state(new ValidatorState);
}

//Validator
void ValidatorState::enter() {
    print("Enter Validator State\n")
}

void ValidatorState::handle() {
    print("Handle Validator State\n");
}

void ValidatorState::exit() {
    print("Exit Validator State\n-------------\n");
    this->context->transition_state(new OutputState);
}

//Output
void OutputState::enter() {
    print("Enter Output State\n");
}

void OutputState::handle() {
    print("Handle Output State\n");
}

void OutputState::exit() {
    print("Exit Output State\n-------------\n");
    this->context->transition_state(new OutputState);
}

//Error
void ErrorState::enter() {
    print("Enter Error State\n")
}

void ErrorState::handle() {
    print("Handle Error State\n");
}

void ErrorState::exit() {
    print("Exit Error State\n-------------\n")
}