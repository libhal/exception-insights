#pragma once
#include <memory>
#include <optional>

#include "state_context.hpp"

/**
 * @class State
 * @brief Abstract base class for all states in the state machine.
 *
 * The State class defines the interface that all concrete states must
 * implement. Each state has three lifecycle phases: enter (initialization),
 * handle (main logic), and exit (cleanup and transition). States use the
 * StateContext to access shared data and determine transitions.
 *
 * Concrete states should inherit from this class and implement all three pure
 * virtual methods to define their specific behavior.
 *
 * @note This is an abstract class and cannot be instantiated directly.
 * @see StateMachine
 * @see StateContext
 * @see state_context.hpp
 */
class State
{
  public:
    /**
     * @brief Called when entering the state.
     *
     * This method is invoked when the state machine transitions into this
     * state. It should perform any initialization or setup required before the
     * state's main logic executes.
     *
     * @param context Reference to the shared state context containing data
     *                and configuration accessible to all states.
     *
     * @note This is called before handle() in the state lifecycle.
     */
    virtual void enter(StateContext& p_context) = 0;

    /**
     * @brief Executes the main logic of the state.
     *
     * This method contains the primary behavior and operations of the state.
     * It is called after enter() and before exit() during state execution.
     *
     * @param context Reference to the shared state context for accessing
     *                and modifying shared data.
     *
     * @note This is where the state performs its core functionality.
     */
    virtual void handle(StateContext& p_context) = 0;

    /**
     * @brief Called when exiting the state and determines the next state.
     *
     * This method is invoked after handle() completes. It should perform any
     * cleanup operations and return the next state to transition to. If this
     * returns std::nullopt, the state machine will terminate.
     *
     * @param context Reference to the shared state context for accessing
     *                data needed to determine the next state.
     *
     * @return std::optional<std::unique_ptr<State>> The next state to
     * transition to, or std::nullopt to terminate the state machine.
     *
     * @note Returning std::nullopt signals the end of state machine execution.
     * @note The returned state's ownership is transferred to the state machine.
     */
    virtual std::optional<std::unique_ptr<State>> exit(StateContext& p_context)
      = 0;

    /**
     * @brief Virtual destructor for proper cleanup of derived classes.
     *
     */
    virtual ~State() = default;
};