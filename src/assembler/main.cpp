#include "assembler/parser.hpp"
#include "backend/turingcompiler.hpp"
#include "output/binarywriter.hpp"
#include "exceptions.hpp"

#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {
    if(argc < 3) {
        std::cerr << "Not enough arguments given" << std::endl;
        return 1;
    }

    std::ifstream input(argv[1]);
    if(!input) {
        std::cerr << "Failed to open input file " << argv[1] << std::endl;
        return 1;
    }

    std::ofstream output(argv[2], std::ofstream::binary);
    if(!output) {
        std::cerr << "Failed to open output file " << argv[2] << std::endl;
        return 1;
    }

    try {
        AssemblyParser parser(input);
        std::vector<Instr> instrs = parser.parse();

        TuringCompiler compiler(instrs.data(), instrs.size());
        TuringMachine machine = compiler.compile();
        BinaryWriter writer(output);

        writer.accept(machine);
    }
    catch(const ProgramException& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}