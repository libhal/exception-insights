#include "../include/concrete_state.hpp"
#include "../include/safe.hpp"

void UserInputState::toggle(Safe* safe) {
    safe->set_state(ElfParserState::get_instance());
}

SafeState& UserInputState::get_instance() {
    static UserInputState instance;
    return instance;
}

void ElfParserState::toggle(Safe* safe) {
    safe->set_state(CallgraphState::get_instance());
}

SafeState& ElfParserState::get_instance() {
    static ElfParserState instance;
    return instance;
}

void CallgraphState::toggle(Safe* safe) {
    safe->set_state(AbiParserState::get_instance());
}

SafeState& CallgraphState::get_instance() {
    static CallgraphState instance;
    return instance;
}

void AbiParserState::toggle(Safe* safe) {
    safe->set_state(OutputState::get_instance());
}

SafeState& AbiParserState::get_instance() {
    static AbiParserState instance;
    return instance;
}

void OutputState::toggle(Safe* safe) {
    safe->set_state(UserInputState::get_instance());
}

SafeState& OutputState::get_instance() {
    static OutputState instance;
    return instance;
}
