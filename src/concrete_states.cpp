#include "../include/concrete_states.hpp"
#include "../include/state.hpp"
#include "../include/state_machine.hpp"

/**
 * @brief Content for each of function is temporary. Please change
 * them according to what the state desires
 *
 */


void UserInputState::enter(StateContext& context) {
    std::print("Previous Number: {}\n", context.get_data());
}

void UserInputState::handle(StateContext& context) {
    context.inc_data();
}

std::optional<std::unique_ptr<State>> UserInputState::exit(StateContext& context) {
    std::ignore = context;
    return std::make_unique<ElfParserState>();
}


void ElfParserState::enter(StateContext& context) {
    std::print("Previous Number: {}\n", context.get_data());
}

void ElfParserState::handle(StateContext& context) {
    context.inc_data();
}

std::optional<std::unique_ptr<State>> ElfParserState::exit(StateContext& context) {
    std::ignore = context;
    return std::make_unique<CallgraphState>();
}


void CallgraphState::enter(StateContext& context) {
    std::print("Previous Number: {}\n", context.get_data());
}

void CallgraphState::handle(StateContext& context) {
    std::ignore = context;
    context.inc_data();
}

std::optional<std::unique_ptr<State>> CallgraphState::exit(StateContext& context) {
    std::ignore = context;
    return std::make_unique<AbiParserState>();
}


void AbiParserState::enter(StateContext& context) {
    std::print("Previous Number: {}\n", context.get_data());
}

void AbiParserState::handle(StateContext& context) {
    context.inc_data();
}

std::optional<std::unique_ptr<State>> AbiParserState::exit(StateContext& context) {
    std::ignore = context;
    return std::make_unique<ValidatorState>();
}


void ValidatorState::enter(StateContext& context) {
    std::print("Previous Number: {}\n", context.get_data());
}

void ValidatorState::handle(StateContext& context) {
    context.inc_data();
}

std::optional<std::unique_ptr<State>> ValidatorState::exit(StateContext& context) {
    std::ignore = context;
    return std::make_unique<OutputState>();
}


void OutputState::enter(StateContext& context) {
    std::print("Previous Number: {}\n", context.get_data());
}

void OutputState::handle(StateContext& context) {
    context.inc_data();
}

std::optional<std::unique_ptr<State>> OutputState::exit(StateContext& context) {
    std::ignore = context;
    return std::nullopt;
}


void ErrorState::enter(StateContext& context) {
    std::ignore = context;
}

void ErrorState::handle(StateContext& context) {
    std::ignore = context;
}

std::optional<std::unique_ptr<State>> ErrorState::exit(StateContext& context) {
    std::ignore = context;
    return std::nullopt;
}