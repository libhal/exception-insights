#include "../include/concrete_states.hpp"
#include "../include/state.hpp"
#include "../include/state_machine.hpp"

/**
 * @brief Content for each of function is temporary. Please change
 * them according to what the state desires
 *
 */


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
    std::ignore = context;
    return new ElfParserState;
}


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
    std::ignore = context;
    return new CallgraphState;
}


void CallgraphState::enter(StateContext& context) {
    std::print("Enter Callgraph State\n");
    std::print("Previous Number: {}\n", context.get_data());
}

void CallgraphState::handle(StateContext& context) {
    std::ignore = context;
    std::print("Handle Callgraph State\n");
}

State* CallgraphState::exit(StateContext& context) {
    std::print("Exit Callgraph State\n-------------\n");
    std::ignore = context;
    return new AbiParserState;
}


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
    std::ignore = context;
    return new ValidatorState;
}


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
    std::ignore = context;
    return new OutputState;
}


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
    std::ignore = context;
    return nullptr;
}


void ErrorState::enter(StateContext& context) {
    std::print("Enter Error State\n");
    std::ignore = context;
}

void ErrorState::handle(StateContext& context) {
    std::print("Handle Error State\n");
    std::ignore = context;
}

State* ErrorState::exit(StateContext& context) {
    std::print("Exit Error State\n-------------\n");
    std::ignore = context;
    return nullptr;
}