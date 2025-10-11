#include "../include/concrete_states.hpp"
#include "../include/state.hpp"
#include "../include/state_machine.hpp"

//User Input
void UserInputState::enter(StateContext& context) {
    std::print("Enter User Input State\n");
    std::print("Previous Number: {}\n", context.get_data());
}

void UserInputState::handle(StateContext& context) {
    std::print("Handle User Input State\n");
    context.inc_data();
}

State* UserInputState::exit(StateContext& context) {
    std::print("Exit User Input State\n-------------\n");
    (void) context;
    return new ElfParserState;
}

//Elf Parser
void ElfParserState::enter(StateContext& context) {
    std::print("Enter Elf Parser State\n");
    std::print("Previous Number: {}\n", context.get_data());
}

void ElfParserState::handle(StateContext& context) {
    std::print("Handle Elf Parser State\n");
    context.inc_data();
}

State* ElfParserState::exit(StateContext& context) {
    std::print("Exit Elf Parser State\n-------------\n");
    (void) context;
    return new CallgraphState;
}

//Callgraph
void CallgraphState::enter(StateContext& context) {
    std::print("Enter Callgraph State\n");
    std::print("Previous Number: {}\n", context.get_data());
}

void CallgraphState::handle(StateContext& context) {
    context.inc_data();
    std::print("Handle Callgraph State\n");
}

State* CallgraphState::exit(StateContext& context) {
    std::print("Exit Callgraph State\n-------------\n");
    (void) context;
    return new AbiParserState;
}

//Abi Parser
void AbiParserState::enter(StateContext& context) {
    std::print("Enter ABI Parser State\n");
    std::print("Previous Number: {}\n", context.get_data());
}

void AbiParserState::handle(StateContext& context) {
    std::print("Handle ABI Parser State\n");
    context.inc_data();
}

State* AbiParserState::exit(StateContext& context) {
    std::print("Exit ABI Parser State\n-------------\n");
    (void) context;
    return new ValidatorState;
}

//Validator
void ValidatorState::enter(StateContext& context) {
    std::print("Enter Validator State\n");
    std::print("Previous Number: {}\n", context.get_data());
}

void ValidatorState::handle(StateContext& context) {
    std::print("Handle Validator State\n");
    context.inc_data();
}

State* ValidatorState::exit(StateContext& context) {
    std::print("Exit Validator State\n-------------\n");
    (void) context;
    return new OutputState;
}

//Output
void OutputState::enter(StateContext& context) {
    std::print("Enter Output State\n");
    std::print("Previous Number: {}\n", context.get_data());
}

void OutputState::handle(StateContext& context) {
    std::print("Handle Output State\n");
    context.inc_data();
}

State* OutputState::exit(StateContext& context) {
    std::print("Exit Output State\n-------------\n");
    (void) context;
    return nullptr;
}

//Error
void ErrorState::enter(StateContext& context) {
    std::print("Enter Error State\n");
    (void) context;
}

void ErrorState::handle(StateContext& context) {
    std::print("Handle Error State\n");
    (void) context;
}

State* ErrorState::exit(StateContext& context) {
    std::print("Exit Error State\n-------------\n");
    (void) context;
    return nullptr;
}