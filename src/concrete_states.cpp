#include "concrete_states.hpp"

/**
 * @brief Content for each of function is temporary. Please change
 * them according to what the state desires
 *
 */

void UserInputState::enter(StateContext& p_context)
{
    std::print("Previous Number: {}\n", p_context.get_data());
}

void UserInputState::handle(StateContext& p_context)
{
    p_context.inc_data();
}

std::optional<std::unique_ptr<State>> UserInputState::exit(
  StateContext& p_context)
{
    std::ignore = p_context;
    return std::make_unique<ElfParserState>();
}

void ElfParserState::enter(StateContext& p_context)
{
    std::print("Previous Number: {}\n", p_context.get_data());
}

void ElfParserState::handle(StateContext& p_context)
{
    p_context.inc_data();
}

std::optional<std::unique_ptr<State>> ElfParserState::exit(
  StateContext& p_context)
{
    std::ignore = p_context;
    return std::make_unique<CallgraphState>();
}

void CallgraphState::enter(StateContext& p_context)
{
    std::print("Previous Number: {}\n", p_context.get_data());
}

void CallgraphState::handle(StateContext& p_context)
{
    std::ignore = p_context;
    p_context.inc_data();
}

std::optional<std::unique_ptr<State>> CallgraphState::exit(
  StateContext& p_context)
{
    std::ignore = p_context;
    return std::make_unique<AbiParserState>();
}

void AbiParserState::enter(StateContext& p_context)
{
    std::print("Previous Number: {}\n", p_context.get_data());
}

void AbiParserState::handle(StateContext& p_context)
{
    p_context.inc_data();
}

std::optional<std::unique_ptr<State>> AbiParserState::exit(
  StateContext& p_context)
{
    std::ignore = p_context;
    return std::make_unique<ValidatorState>();
}

void ValidatorState::enter(StateContext& p_context)
{
    std::print("Previous Number: {}\n", p_context.get_data());
}

void ValidatorState::handle(StateContext& p_context)
{
    p_context.inc_data();
}

std::optional<std::unique_ptr<State>> ValidatorState::exit(
  StateContext& p_context)
{
    std::ignore = p_context;
    return std::make_unique<OutputState>();
}

void OutputState::enter(StateContext& p_context)
{
    std::print("Previous Number: {}\n", p_context.get_data());
}

void OutputState::handle(StateContext& p_context)
{
    p_context.inc_data();
}

std::optional<std::unique_ptr<State>> OutputState::exit(StateContext& p_context)
{
    std::ignore = p_context;
    return std::nullopt;
}

void ErrorState::enter(StateContext& p_context)
{
    std::ignore = p_context;
}

void ErrorState::handle(StateContext& p_context)
{
    std::ignore = p_context;
}

std::optional<std::unique_ptr<State>> ErrorState::exit(StateContext& p_context)
{
    std::ignore = p_context;
    return std::nullopt;
}