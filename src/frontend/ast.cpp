#include "frontend/ast.hpp"

#include <iostream>

ASTNode::ASTNode(NodeType type, const std::vector<ASTNode*>& children) : type(type), children(children) {}
ASTNode::ASTNode(NodeType type, const std::vector<ASTNode*>& children, DataType datatype) : type(type), children(children), datatype(datatype) {}
ASTNode::ASTNode(NodeType type, const std::vector<ASTNode*>& children, DataType datatype, const std::string& str) : type(type), children(children), datatype(datatype), str(str) {}
ASTNode::ASTNode(NodeType type, const std::vector<ASTNode*>& children, DataType datatype, uint64_t integer) : type(type), children(children), datatype(datatype), integer(integer) {}
ASTNode::ASTNode(NodeType type, const std::vector<ASTNode*>& children, DataType datatype, uint64_t integer, uint64_t integer2) : type(type), children(children), datatype(datatype), integer(integer), integer2(integer2) {}
ASTNode::ASTNode(NodeType type, uint64_t integer) : type(type), integer(integer) {}

ASTNode::~ASTNode() {
    for(ASTNode* c : this->children)
        delete c;
}

std::ostream& operator<<(std::ostream& os, DataType type) {
    const char* msg;
    switch(type) {
        case DataType::INVALID:
            msg = "invalid";
            break;
        case DataType::VOID:
            msg = "void";
            break;
        case DataType::U8:
            msg = "u8";
            break;
        case DataType::U16:
            msg = "u16";
            break;
        case DataType::U32:
            msg = "u32";
            break;
        default:
            msg = "(unknown)";
            break;
    }
    os << msg;
    return os;
}

size_t datatype_size(DataType type) {
    switch(type) {
        case DataType::INVALID:
        case DataType::VOID:
            return 0;
        case DataType::U8:
            return 1;
        case DataType::U16:
            return 2;
        case DataType::U32:
            return 4;
    }
    return 0;
}