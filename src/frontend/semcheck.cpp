#include "frontend/semcheck.hpp"
#include "exceptions.hpp"

#include <stdexcept>
#include <sstream>
#include <unordered_set>
#include <algorithm>

std::unordered_set<DataType> ALL_DATA_TYPES = {DataType::VOID, DataType::U8, DataType::U16, DataType::U32};
std::unordered_set<DataType> INTEGER_DATA_TYPES = {DataType::U8, DataType::U16, DataType::U32};
std::unordered_set<DataType> EXPRESSION_DATA_TYPES = {DataType::VOID, DataType::U8, DataType::U16, DataType::U32};
std::unordered_set<DataType> ARITH_DATA_TYPES = {DataType::U8, DataType::U16, DataType::U32};

template <typename T>
std::unordered_set<T> intersect_unordered(const std::unordered_set<T>& a, const std::unordered_set<T>& b) {
    std::unordered_set<T> result;

    for(const T& e : a) {
        if(b.count(e) > 0)
            result.insert(e);
    }
    return result;
}

SemanticChecker::SemanticChecker(ASTNode* ast) : ast(ast) {}

std::unordered_set<DataType> SemanticChecker::getTypeOptions(ASTNode* node) {
    switch(node->type) {
        case NodeType::INT_CONST:
            return INTEGER_DATA_TYPES;
        case NodeType::U8_INT_CONST:
            return {DataType::U8};
        case NodeType::U16_INT_CONST:
            return {DataType::U16};
        case NodeType::U32_INT_CONST:
            return {DataType::U32};
        case NodeType::ASSIGN_EXPR:
        case NodeType::ARRAY_ASSIGN_CONST: {
            std::unordered_set<DataType> result = {node->datatype};
            for(ASTNode* child : node->children) {
                auto new_set = this->getTypeOptions(child);
                result = intersect_unordered(result, new_set);
            }
            return result;
        }
        case NodeType::ID_EXPR:
        case NodeType::SUBSCRIPT_CONST:
            return {node->datatype};
        case NodeType::SUBSCRIPT_INDR:
            this->fitExprRoot(node->children[0], {DataType::U32});
            return {node->datatype};
        case NodeType::CAST_EXPR:
            this->fitExprRoot(node->children[0], ARITH_DATA_TYPES);
            return {node->datatype};
        case NodeType::ARRAY_ASSIGN_INDR: {
            this->fitExprRoot(node->children[0], {DataType::U32});
            std::unordered_set<DataType> result = {node->datatype};
            auto new_set = this->getTypeOptions(node->children[1]);
            result = intersect_unordered(result, new_set);
            return result;
        }
        default: {
            auto result = ALL_DATA_TYPES;

            for(ASTNode* child : node->children) {
                auto new_set = this->getTypeOptions(child);
                result = intersect_unordered(result, new_set);
            }
            return result;
        }
    }
}

void SemanticChecker::setTypes(ASTNode* node, DataType type) {
    node->datatype = type;
    switch(node->type) {
        case NodeType::SUBSCRIPT_INDR:
        case NodeType::CAST_EXPR:
            break;
        case NodeType::ARRAY_ASSIGN_INDR:
            this->setTypes(node->children[1], type);
            break;
        default:
            for(ASTNode* c : node->children)
                this->setTypes(c, type);
            break;
    }
}

void SemanticChecker::fitExprRoot(ASTNode* node, const std::unordered_set<DataType>& valid_types) {
    auto options = this->getTypeOptions(node);
    if(options.size() == 0) {
        // Fit types as far as possible
        node->datatype = DataType::INVALID;

        for(ASTNode* c : node->children)
            this->fitExprRoot(c, valid_types);
        return;
    }
    if(options.size() > 1) {
        options = intersect_unordered(options, valid_types);
    }
    if(options.size() > 1) {
        std::stringstream ss;
        ss << "Type for expression is ambiguous, candidates: ";
        bool first = true;
        for(DataType d : options) {
            if(first)
                first = false;
            else
                ss << ", ";

            ss << d;
        }
        throw ProgramException(ss.str());
    }

    this->setTypes(node, *options.begin());
}

void SemanticChecker::fitNode(ASTNode* node) {
    switch(node->type) {
        case NodeType::EXPR_STAT:
            this->fitExprRoot(node->children[0], ALL_DATA_TYPES);
            break;
        case NodeType::IF_STAT:
        case NodeType::IF_ELSE_STAT:
        case NodeType::WHILE_STAT:
            this->fitExprRoot(node->children[0], {DataType::U8});
            for(size_t i = 1; i < node->children.size(); ++i)
                this->fitNode(node->children[i]);
            break;
        default:
            for(ASTNode* n : node->children)
                this->fitNode(n);
            break;
    }
}

void SemanticChecker::checkNode(ASTNode* node) {
    for(ASTNode* c : node->children) {
        this->checkNode(c);
    }

    auto assert_type_of = [](DataType type, const std::unordered_set<DataType>& candidates) {
        if(candidates.count(type) == 0) {
            std::stringstream ss;
            ss << "Invalid type for node: expected ";
            bool first = true;
            for(DataType d : candidates) {
                if(first)
                    first = false;
                else
                    ss << ", ";
                ss << d;
            }
            ss << "; got " << type;
            throw ProgramException(ss.str());
        }
    };
    auto assert_type_same = [](DataType t1, DataType t2) {
        if(t1 != t2) {
            std::stringstream ss;
            ss << "Type mismatch: " << t1 << " and " << t2;
            throw ProgramException(ss.str());
        }
    };

    switch(node->type) {
        case NodeType::EMPTY:
        case NodeType::LIST:
        case NodeType::FUNC_DECL:
        case NodeType::GLOBAL_DECL:
            break;
        case NodeType::EXPR_STAT:
            assert_type_of(node->children[0]->datatype, EXPRESSION_DATA_TYPES);
            break;
        case NodeType::IF_STAT:
        case NodeType::IF_ELSE_STAT:
        case NodeType::WHILE_STAT:
            assert_type_of(node->children[0]->datatype, {DataType::U8});
            break;
        case NodeType::ADD_EXPR:
        case NodeType::SUB_EXPR:
        case NodeType::AND_EXPR:
        case NodeType::OR_EXPR:
        case NodeType::XOR_EXPR:
            assert_type_of(node->children[0]->datatype, ARITH_DATA_TYPES);
            assert_type_of(node->children[1]->datatype, ARITH_DATA_TYPES);
            assert_type_same(node->children[0]->datatype, node->children[1]->datatype);
            break;
        case NodeType::ASSIGN_EXPR:
        case NodeType::ARRAY_ASSIGN_CONST:
            assert_type_of(node->children[0]->datatype, ARITH_DATA_TYPES);
            assert_type_same(node->datatype, node->children[0]->datatype);
            break;
        case NodeType::SUBSCRIPT_INDR:
            assert_type_of(node->children[0]->datatype, {DataType::U32});
            break;
        case NodeType::ARRAY_ASSIGN_INDR:
            assert_type_of(node->children[0]->datatype, {DataType::U32});
            assert_type_of(node->children[1]->datatype, ARITH_DATA_TYPES);
            assert_type_same(node->datatype, node->children[1]->datatype);
            break;
        case NodeType::CAST_EXPR:
            assert_type_of(node->datatype, ARITH_DATA_TYPES);
            assert_type_of(node->children[0]->datatype, ARITH_DATA_TYPES);
            break;
        case NodeType::ID_EXPR:
        case NodeType::SUBSCRIPT_CONST:
        case NodeType::INT_CONST:
        case NodeType::U8_INT_CONST:
        case NodeType::U16_INT_CONST:
        case NodeType::U32_INT_CONST:
            break;
    }
}

void SemanticChecker::check() {
    this->fitNode(this->ast);
    this->checkNode(this->ast);
}