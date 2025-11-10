#include "state_machine.hpp"
#include "concrete_states.hpp"

StateMachine::StateMachine()
{
    m_current_state = std::make_unique<UserInputState>();
}

std::optional<std::reference_wrapper<State>> StateMachine::get_current_state()
{
    if (m_current_state) {
        return std::ref(**m_current_state);
    }
    return std::nullopt;
}

StateContext& StateMachine::get_context()
{
    return m_context;
}

void StateMachine::run_state()
{
    while (m_current_state != std::nullopt) {
        m_current_state.value()->enter(m_context);
        m_current_state.value()->handle(m_context);
        transition_state(m_current_state.value()->exit(m_context));
    }
}

void StateMachine::transition_state(
  std::optional<std::unique_ptr<State>> new_state)
{
    this->m_current_state = std::move(new_state);
}