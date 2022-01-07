#include <iostream>
#include <fstream>

#include "frontend/parser.hpp"
#include "turingc.parse.h"
#include "turingc.lex.h"

#include "frontend/ast.hpp"
#include "frontend/semcheck.hpp"
#include "frontend/asmgen.hpp"
#include "frontend/symtab.hpp"

#include "backend/turingcompiler.hpp"
#include "output/binarywriter.hpp"
#include "exceptions.hpp"

void yyerror(void* scanner, parse_info* parser, const char* msg) {
    std::cerr << "Error: " << msg << std::endl;
}

int main(int argc, char* argv[]) {
    if(argc < 3) {
        std::cerr << "Not enough arguments given" << std::endl;
        return 1;
    }

    FILE* file = std::fopen(argv[1], "rb");
    if(!file) {
        std::cerr << "Failed to open file " << argv[1] << std::endl;
        return 1;
    }

    std::ofstream output(argv[2]);
    if(!output) {
        std::cerr << "Failed to create file " << argv[2] << std::endl;
        return 1;
    }

    parse_info parser;
    yyscan_t lexer;

    parser.ast = nullptr;
    parser.symtab = new Symtab();

    yylex_init(&lexer);
    yyset_in(file, lexer);
    yyset_out(NULL, lexer);
    int error = yyparse(lexer, &parser);
    yylex_destroy(lexer);

    if(error)
        return 1;

    ASTNode* root = parser.ast;
    try {
        SemanticChecker checker(root);
        checker.check();

        AsmGenerator generator(root, parser.symtab);
        auto instrs = generator.run();
        for(const auto& instr : instrs) {
            std::cout << instr << std::endl;
        }

        TuringCompiler compiler(&instrs[0], instrs.size());
        TuringMachine machine = compiler.compile();

        BinaryWriter writer(output);
        writer.accept(machine);
    }
    catch(const ProgramException& err) {
        std::cerr << "Compile error: " << err.what() << std::endl;
        delete root;
        delete parser.symtab;
        return 1;
    }

    delete root;
    delete parser.symtab;

    return 0;
}