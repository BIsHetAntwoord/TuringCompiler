#ifndef _TURINGCOMPILER_FRONTEND_AST_HPP
#define _TURINGCOMPILER_FRONTEND_AST_HPP

#include <vector>
#include <cstdint>
#include <iosfwd>
#include <string>

enum class NodeType {
    EMPTY,
    LIST,

    FUNC_DECL,
    GLOBAL_DECL,

    EXPR_STAT,
    IF_STAT,
    IF_ELSE_STAT,
    WHILE_STAT,

    ADD_EXPR,
    SUB_EXPR,
    AND_EXPR,
    OR_EXPR,
    XOR_EXPR,
    ASSIGN_EXPR,
    ID_EXPR,
    CAST_EXPR,
    SUBSCRIPT_CONST,
    SUBSCRIPT_INDR,
    ARRAY_ASSIGN_CONST,
    ARRAY_ASSIGN_INDR,

    INT_CONST,
    U8_INT_CONST,
    U16_INT_CONST,
    U32_INT_CONST
};

enum class DataType {
    INVALID,
    VOID,
    U8,
    U16,
    U32
};

struct ASTNode {
    NodeType type;
    std::vector<ASTNode*> children;
    DataType datatype = DataType::INVALID;
    uint64_t integer, integer2;
    std::string str;

    ASTNode(NodeType, const std::vector<ASTNode*>&);
    ASTNode(NodeType, const std::vector<ASTNode*>&, DataType);
    ASTNode(NodeType, const std::vector<ASTNode*>&, DataType, const std::string&);
    ASTNode(NodeType, const std::vector<ASTNode*>&, DataType, uint64_t);
    ASTNode(NodeType, const std::vector<ASTNode*>&, DataType, uint64_t, uint64_t);
    ASTNode(NodeType, uint64_t);
    ~ASTNode();
};

std::ostream& operator<<(std::ostream&, DataType);
size_t datatype_size(DataType);

#endif
