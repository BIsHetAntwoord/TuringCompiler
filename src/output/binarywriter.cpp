#include "output/binarywriter.hpp"

#include <iostream>

BinaryWriter::BinaryWriter(std::ostream& output) : output(output) {}

void BinaryWriter::accept(const TuringMachine& machine) {
    this->write<uint64_t>(machine.start_state);
    this->write<uint64_t>(machine.accept_state);
    this->write<uint64_t>(machine.reject_state);

    uint64_t num_states = machine.states.size();
    this->write<uint64_t>(num_states);

    for(uint64_t i = 0; i < num_states; ++i) {
        const TuringState& state = machine.states[i];

        uint64_t num_trans = state.transitions.size();

        this->write<uint64_t>(i);
        this->write<uint64_t>(num_trans);

        this->write<uint64_t>(state.def_transition.output);
        this->write<uint8_t>((uint8_t)state.def_transition.dir);
        this->write<uint64_t>(state.def_transition.next_state);

        for(uint64_t j = 0; j < num_trans; ++j) {
            this->write<uint64_t>(state.transitions[j].input);
            this->write<uint64_t>(state.transitions[j].output);
            this->write<uint8_t>((uint8_t)state.transitions[j].dir);
            this->write<uint64_t>(state.transitions[j].next_state);
        }
    }
}