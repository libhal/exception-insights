#pragma once
#include <memory>
#include <optional>
#include <print>

#include "concrete_states.hpp"
#include "state.hpp"
#include "state_context.hpp"

class State;

/**
 * @class StateMachine
 * @brief Manages state transitions and execution in a finite state machine.
 *
 * The StateMachine class implements a complete state machine that automatically
 * executes states in sequence. Each state performs its enter, handle, and exit
 * operations, with the exit operation determining the next state transition.
 * The machine runs until a state returns std::nullopt, indicating completion.
 *
 * @note The state machine begins with UserInputState as the initial state.
 * @note States are owned exclusively by the state machine via unique pointers.
 * @see State
 * @see StateContext
 * @see concrete_states.hpp
 */
class StateMachine
{
  public:
    /**
     * @brief Constructs a new StateMachine with UserInputState as the initial
     * state.
     *
     * Initializes the state machine with a default context and sets
     * UserInputState as the starting state.
     */
    StateMachine();

    /**
     * @brief Destroys the StateMachine.
     *
     * Default destructor that cleans up the current state and context.
     */
    ~StateMachine() = default;

    /**
     * @brief Gets a reference to the current state.
     *
     * Retrieves the currently active state without transferring ownership.
     * The reference remains valid until the state transitions.
     *
     * @return std::optional<std::reference_wrapper<State>> A reference to
     *         the current state, or std::nullopt if no state is active.
     *
     * @note This dereferences the optional and unique_ptr to return a direct
     *       reference to the State object.
     */
    std::optional<std::reference_wrapper<State>> get_current_state();

    /**
     * @brief Gets the reference of the current state context.
     *
     * Retrieves the reference of the context containing shared data and
     * configuration used by states during execution.
     *
     * @return StateContext A reference of the current context.
     */
    StateContext& get_context();

    /**
     * @brief Executes the state machine until completion.
     *
     * Runs a continuous loop that executes each state's full lifecycle:
     * 1. Calls enter() on the current state
     * 2. Calls handle() to perform the state's main logic
     * 3. Calls exit() which returns the next state (or nullopt to terminate)
     * 4. Transitions to the next state
     *
     * The loop continues until a state's exit() method returns std::nullopt,
     * indicating the state machine has completed its execution.
     *
     * @note This is a blocking operation that runs until the state machine
     *       reaches a terminal state.
     * @see State::enter()
     * @see State::handle()
     * @see State::exit()
     */
    void run_state();

    /**
     * @brief Transitions to a new state by moving ownership.
     *
     * Replaces the current state with the new state. If p_new_state is
     * std::nullopt, the state machine will have no active state and
     * run_state() will terminate.
     *
     * @param p_new_state The new state to transition to, or std::nullopt to
     *                  terminate the state machine.
     *
     * @note Ownership of the new state is transferred to the state machine
     *       using move semantics.
     * @note The previous state is automatically destroyed when replaced.
     */
    void transition_state(std::optional<std::unique_ptr<State>> p_new_state);

  private:
    /**
     * @brief The currently active state.
     *
     * Holds exclusive ownership of the current state. Set to std::nullopt
     * when the state machine has completed execution.
     */
    std::optional<std::unique_ptr<State>> m_current_state;

    /**
     * @brief The context shared across all states.
     *
     * Contains data and configuration that persists across state transitions.
     * This context is passed to each state's enter(), handle(), and exit()
     * methods.
     */
    StateContext m_context;
};
