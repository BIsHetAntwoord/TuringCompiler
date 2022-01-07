#ifndef _TURINGCOMPILER_FRONTEND_ASMGEN_HPP
#define _TURINGCOMPILER_FRONTEND_ASMGEN_HPP

#include "backend/instr.hpp"

#include <vector>
#include <unordered_map>

class ASTNode;
class Symtab;

class AsmGenerator {
    private:
        ASTNode* root;
        Symtab* symtab;
        std::unordered_map<std::string, size_t> labels;
        std::vector<Instr> instrs;

        size_t label_offset = 0;

        void generateGlobals(ASTNode*);
        void generateGlobal(ASTNode*);
        void generateFunctions(ASTNode*);
        void generate(ASTNode*);

        std::string nextLabelName();
    public:
        AsmGenerator(ASTNode*, Symtab*);

        std::vector<Instr> run();
};

#endif
