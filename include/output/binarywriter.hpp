#ifndef _TURINGCOMPILER_OUTPUT_BINARYWRITER_HPP
#define _TURINGCOMPILER_OUTPUT_BINARYWRITER_HPP

#include "backend/turingstate.hpp"

#include <iostream>

class BinaryWriter {
    private:
        std::ostream& output;

        template <typename T>
        void write(const T&);
    public:
        BinaryWriter(std::ostream&);

        void accept(const TuringMachine&);
};

template <typename T>
void BinaryWriter::write(const T& value) {
    this->output.write((const char*)&value, sizeof(T));
}

#endif
