#ifndef _TURINGCOMPILER_BACKEND_TURINGSTAGE_HPP
#define _TURINGCOMPILER_BACKEND_TURINGSTAGE_HPP

#include <cstddef>
#include <limits>
#include <iosfwd>
#include <vector>

const size_t TRANS_WILDCARD = std::numeric_limits<size_t>::max();
const size_t TAPE_BP = 256;
const size_t TAPE_AP = 257;
const size_t TAPE_TEMP1 = 258;
const size_t TAPE_GP = 259;

enum class TuringDirection {
    STAY,
    LEFT,
    RIGHT
};

struct TuringTransition {
    size_t input;
    size_t output;
    TuringDirection dir;
    size_t next_state;
};

struct TuringState {
    std::vector<TuringTransition> transitions;
    TuringTransition def_transition;
};

struct TuringMachine {
    size_t start_state;
    size_t accept_state;
    size_t reject_state;
    std::vector<TuringState> states;
};

std::ostream& operator<<(std::ostream&, const TuringDirection&);
std::ostream& operator<<(std::ostream&, const TuringTransition&);

#endif
