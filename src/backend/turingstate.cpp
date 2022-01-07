#include "backend/turingstate.hpp"

#include <iostream>

std::ostream& operator<<(std::ostream& os, const TuringDirection& dir) {
    switch(dir) {
        case TuringDirection::STAY:
            os << "!";
            break;
        case TuringDirection::LEFT:
            os << "<";
            break;
        case TuringDirection::RIGHT:
            os << ">";
            break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const TuringTransition& trans) {
    if(trans.input == TRANS_WILDCARD)
        os << "*";
    else
        os << trans.input;

    os << " ";
    if(trans.output == TRANS_WILDCARD)
        os << "*";
    else
        os << trans.output;

    os << " " << trans.dir << " -> " << trans.next_state;
    return os;
}