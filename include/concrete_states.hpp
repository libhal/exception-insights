/**
 * @file concrete_states.hpp
 * @brief Concrete state implementations for the state machine.
 *
 * This file defines all concrete states used in the state machine workflow.
 * Each state represents a distinct phase in the processing pipeline, from
 * user input through validation to output generation.
 *
 * The states follow a sequential workflow:
 * UserInput → ElfParser → Callgraph → AbiParser → Validator → Output
 *
 * ErrorState can be entered from any state when an error occurs.
 */

#pragma once
#include <memory>
#include <optional>
#include <print>

#include "state.hpp"
#include "state_context.hpp"

/**
 * @class UserInputState
 * @brief Initial state that handles user input collection and validation.
 *
 * This is the entry point of the state machine. It processes user input,
 * validates parameters, and prepares the context for subsequent states.
 *
 * @note This is the default initial state of the StateMachine.
 */
class UserInputState : public State
{
  public:
    /**
     * @brief Initialize user input processing.
     * @param p_context Shared state context for storing input data.
     */
    void enter(StateContext& p_context) override;

    /**
     * @brief Process and validate user input.
     * @param p_context Context to store validated input parameters.
     */
    void handle(StateContext& p_context) override;

    /**
     * @brief Transition to ElfParserState or ErrorState.
     * @param p_context Context containing validated input.
     * @return Next state (typically ElfParserState) or ErrorState on failure.
     */
    std::optional<std::unique_ptr<State>> exit(
      StateContext& p_context) override;
};

/**
 * @class ElfParserState
 * @brief Parses ELF (Executable and Linkable Format) binary files.
 *
 * This state reads and analyzes ELF binaries, extracting relevant information
 * such as symbols, sections, and metadata needed for further processing.
 */
class ElfParserState : public State
{
  public:
    /**
     * @brief Initialize ELF parsing resources.
     * @param p_context Context containing file paths and configuration.
     */
    void enter(StateContext& p_context) override;

    /**
     * @brief Parse the ELF binary and extract data.
     * @param p_context Context to store parsed ELF information.
     */
    void handle(StateContext& p_context) override;

    /**
     * @brief Transition to CallgraphState or ErrorState.
     * @param p_context Context containing parsed ELF data.
     * @return Next state (typically CallgraphState) or ErrorState on parse
     * failure.
     */
    std::optional<std::unique_ptr<State>> exit(
      StateContext& p_context) override;
};

/**
 * @class CallgraphState
 * @brief Generates and analyzes the call graph from parsed binary data.
 *
 * This state parses the GCC call graph representing function relationships
 * and dependencies within the analyzed binary.
 */
class CallgraphState : public State
{
  public:
    /**
     * @brief Initialize call graph generation.
     * @param p_context Context containing parsed binary data.
     */
    void enter(StateContext& p_context) override;

    /**
     * @brief Build the call graph structure.
     * @param p_context Context to store the generated call graph.
     */
    void handle(StateContext& p_context) override;

    /**
     * @brief Transition to AbiParserState or ErrorState.
     * @param p_context Context containing the call graph.
     * @return Next state (typically AbiParserState) or ErrorState on failure.
     */
    std::optional<std::unique_ptr<State>> exit(
      StateContext& p_context) override;
};

/**
 * @class AbiParserState
 * @brief Parses and analyzes Application Binary Interface (ABI) information.
 *
 * This state extracts and processes ABI-related data, including function
 * signatures, calling conventions, and interface specifications.
 */
class AbiParserState : public State
{
  public:
    /**
     * @brief Initialize ABI parsing.
     * @param p_context Context containing call graph and binary data.
     */
    void enter(StateContext& p_context) override;

    /**
     * @brief Parse ABI specifications and interface data.
     * @param p_context Context to store parsed ABI information.
     */
    void handle(StateContext& p_context) override;

    /**
     * @brief Transition to ValidatorState or ErrorState.
     * @param p_context Context containing ABI data.
     * @return Next state (typically ValidatorState) or ErrorState on parse
     * failure.
     */
    std::optional<std::unique_ptr<State>> exit(
      StateContext& p_context) override;
};

/**
 * @class ValidatorState
 * @brief Validates the processed data for correctness and consistency.
 *
 * This state performs validation checks on the parsed and processed data
 * to ensure exceptions meets requirements and is internally consistent before
 * output.
 */
class ValidatorState : public State
{
  public:
    /**
     * @brief Initialize validation checks.
     * @param p_context Context containing all processed data.
     */
    void enter(StateContext& p_context) override;

    /**
     * @brief Perform validation on processed data.
     * @param p_context Context containing data to validate.
     */
    void handle(StateContext& p_context) override;

    /**
     * @brief Transition to OutputState or ErrorState.
     * @param p_context Context with validation results.
     * @return Next state (OutputState if valid) or ErrorState on validation
     * failure.
     */
    std::optional<std::unique_ptr<State>> exit(
      StateContext& p_context) override;
};

/**
 * @class OutputState
 * @brief Generates and writes the final output.
 *
 * This is the final state in the normal execution flow. It formats and
 * outputs the processed results to the specified destination.
 */
class OutputState : public State
{
  public:
    /**
     * @brief Initialize output generation.
     * @param p_context Context containing validated data to output.
     */
    void enter(StateContext& p_context) override;

    /**
     * @brief Generate and write output.
     * @param p_context Context containing all processed data.
     */
    void handle(StateContext& p_context) override;

    /**
     * @brief Complete execution and terminate state machine.
     * @param p_context Context with output results.
     * @return std::nullopt to signal state machine completion.
     */
    std::optional<std::unique_ptr<State>> exit(
      StateContext& p_context) override;
};

/**
 * @class ErrorState
 * @brief Handles error conditions and cleanup.
 *
 * This state is entered when an error occurs in any other state. It handles
 * error reporting, logging, and cleanup operations before terminating the
 * state machine.
 *
 * @note This state always returns std::nullopt to terminate execution.
 */
class ErrorState : public State
{
  public:
    /**
     * @brief Initialize error handling.
     * @param p_context Context containing error information.
     */
    void enter(StateContext& p_context) override;

    /**
     * @brief Process and report the error.
     * @param p_context Context with error details for reporting.
     */
    void handle(StateContext& p_context) override;

    /**
     * @brief Terminate state machine after error handling.
     * @param p_context Context after error processing.
     * @return std::nullopt to signal state machine termination.
     */
    std::optional<std::unique_ptr<State>> exit(
      StateContext& p_context) override;
};