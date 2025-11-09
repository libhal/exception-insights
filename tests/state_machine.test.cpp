#include "state_machine.hpp"
#include "concrete_states.hpp"
#include "state.hpp"
#include "state_context.hpp"
#include <boost/ut.hpp>
#include <memory>
#include <sstream>
#include <type_traits>
#include <optional>
#include <typeindex>
#include <vector>

// Mock State for testing
class MockState : public State
{
  public:
    bool enter_called = false;
    bool handle_called = false;
    bool exit_called = false;

    void enter(StateContext& context) override
    {
        std::ignore = context;
        enter_called = true;
    }

    void handle(StateContext& context) override
    {
        std::ignore = context;
        handle_called = true;
    }

    std::optional<std::unique_ptr<State>> exit(StateContext& context) override
    {
        std::ignore = context;
        exit_called = true;
        return std::nullopt;
    }
};

void state_machine_tests()
{
    using namespace boost::ut;

    "StateMachine"_test = [] {
        "Initalize inital state"_test = [] {
            StateMachine sm;
            auto actual_state = sm.get_current_state();
            expect(actual_state.has_value()) << "A State is initialized";

            State& actual_state_ref = actual_state.value().get();
            expect(typeid(actual_state_ref) == typeid(UserInputState)) << "Initial state should be UserInputState";
        };

        "Transition State"_test = [] {
            StateMachine sm;
            std::optional<std::unique_ptr<State>> mock_state = std::make_unique<MockState>();
            sm.transition_state(std::move(mock_state));

            auto actual_state = sm.get_current_state();
            expect(actual_state.has_value()) << "A State is initialized";

            State& actual_state_ref = actual_state.value().get();
            expect(typeid(actual_state_ref) == typeid(MockState)) << "State should transition to MockState";
        };

        "State calling enter, handle, and exit"_test = [] {
            StateMachine sm;
            auto mock_state = std::make_unique<MockState>();
            auto* mock_ptr = mock_state.get();
            
            sm.transition_state(std::move(mock_state));
            
            auto state = sm.get_current_state();
            if (state.has_value()) {
                StateContext context;
                state.value().get().enter(context);
                state.value().get().handle(context);
                state.value().get().exit(context);
            }
            
            expect(mock_ptr->enter_called) << "Should run enter()";
            expect(mock_ptr->handle_called) << "Should run handle()";
            expect(mock_ptr->exit_called) << "Should run exit()";
        };

        "State Transition Order"_test = [] {
            StateMachine sm;
            StateContext context = sm.get_context();
            std::optional<std::reference_wrapper<State>> actual_state;
            std::vector<std::type_index> state_order = {
                typeid(UserInputState),
                typeid(ElfParserState),
                typeid(CallgraphState),
                typeid(AbiParserState),
                typeid(ValidatorState),
                typeid(OutputState)
            };
            
            for (auto& expect_state : state_order){
                actual_state = sm.get_current_state();
                State& actual_state_ref = actual_state.value().get();
                expect(typeid(actual_state_ref) == expect_state) << "State should transition to MockState";
                actual_state_ref.enter(context);
                actual_state_ref.handle(context);
                sm.transition_state(actual_state_ref.exit(context));
            }
        };
    };
}