#ifndef _TURINGCOMPILER_FRONTEND_SEMCHECK_HPP
#define _TURINGCOMPILER_FRONTEND_SEMCHECK_HPP

#include "frontend/ast.hpp"

#include <unordered_set>

class SemanticChecker {
    private:
        ASTNode* ast;

        std::unordered_set<DataType> getTypeOptions(ASTNode*);
        void setTypes(ASTNode*, DataType);
        void fitExprRoot(ASTNode*, const std::unordered_set<DataType>&);
        void fitNode(ASTNode*);
        void checkNode(ASTNode*);
    public:
        SemanticChecker(ASTNode*);

        void check();
};

#endif
