#include "frontend/asmgen.hpp"
#include "frontend/ast.hpp"
#include "frontend/symtab.hpp"

#include <sstream>
#include <stdexcept>
#include <bit>

inline Opcode overload_size(DataType type, Opcode op_base) {
    return static_cast<Opcode>(static_cast<size_t>(op_base) + static_cast<size_t>(type) - static_cast<size_t>(DataType::U8));
}

inline Opcode get_overloaded_op(NodeType type, DataType data_type) {
    switch(type) {
        case NodeType::ADD_EXPR:
            return overload_size(data_type, Opcode::ADD8);
        case NodeType::SUB_EXPR:
            return overload_size(data_type, Opcode::SUB8);
        case NodeType::AND_EXPR:
            return overload_size(data_type, Opcode::AND8);
        case NodeType::OR_EXPR:
            return overload_size(data_type, Opcode::OR8);
        case NodeType::XOR_EXPR:
            return overload_size(data_type, Opcode::XOR8);
        default:
            return Opcode::REJECT;
    }
}

AsmGenerator::AsmGenerator(ASTNode* root, Symtab* symtab) : root(root), symtab(symtab) {}

std::string AsmGenerator::nextLabelName() {
    std::stringstream result;
    result << "$internal.";
    result << this->label_offset++;
    return result.str();
}

void AsmGenerator::generateGlobals(ASTNode* node) {
    if(node->type == NodeType::GLOBAL_DECL) {
        this->generate(node);
    }
    else {
        for(ASTNode* c : node->children)
            this->generateGlobals(c);
    }
}

void AsmGenerator::generateGlobal(ASTNode* node) {
    size_t global_size = this->symtab->getGlobalSpaceSize();
    if(global_size > 0)
        this->instrs.push_back(make_instr(Opcode::ALLOC, this->symtab->getGlobalSpaceSize()));

    this->generateGlobals(node);
}

void AsmGenerator::generateFunctions(ASTNode* node) {
    if(node->type == NodeType::FUNC_DECL) {
        this->labels[node->str] = this->instrs.size();
        this->generate(node);
    }
    else {
        for(ASTNode* n : node->children)
            this->generateFunctions(n);
    }
}

void AsmGenerator::generate(ASTNode* node) {
    switch(node->type) {
        case NodeType::EMPTY:
            break;
        case NodeType::LIST:
            for(ASTNode* c : node->children)
                this->generate(c);
            break;
        case NodeType::FUNC_DECL: {
            Instr enter_scope = make_instr(Opcode::ENTER);
            this->instrs.push_back(enter_scope);

            size_t local_stack_size = this->symtab->getFunctionLocalSize(node->str);
            if(local_stack_size > 0) {
                Instr alloc = make_instr(Opcode::ALLOC, local_stack_size);
                this->instrs.push_back(alloc);
            }

            this->generate(node->children[0]);

            std::string func_ret_label = "$" + node->str + ".ret";
            this->labels[func_ret_label] = this->instrs.size();

            Instr ret = make_instr(Opcode::RET);
            this->instrs.push_back(ret);
            break;
        }
        case NodeType::GLOBAL_DECL:
            this->generate(node->children[0]);
            break;
        case NodeType::EXPR_STAT: {
            this->generate(node->children[0]);
            DataType child_type = node->children[0]->datatype;
            if(child_type != DataType::VOID) {
                Instr dealloc = make_instr(overload_size(child_type, Opcode::POP8));
                this->instrs.push_back(dealloc);
            }
            break;
        }
        case NodeType::IF_STAT: {
            this->generate(node->children[0]);
            Instr jmp = make_instr(Opcode::JF, this->nextLabelName());
            this->instrs.push_back(jmp);

            this->generate(node->children[1]);
            this->labels[jmp.label] = this->instrs.size();
            break;
        }
        case NodeType::IF_ELSE_STAT: {
            this->generate(node->children[0]);
            Instr jmp = make_instr(Opcode::JF, this->nextLabelName());
            this->instrs.push_back(jmp);

            this->generate(node->children[1]);
            Instr jmp_2 = make_instr(Opcode::JMP, this->nextLabelName());
            this->instrs.push_back(jmp_2);
            this->labels[jmp.label] = this->instrs.size();
            this->generate(node->children[2]);
            this->labels[jmp_2.label] = this->instrs.size();
            break;
        }
        case NodeType::WHILE_STAT: {
            std::string cond_label = this->nextLabelName();
            this->labels[cond_label] = this->instrs.size();
            this->generate(node->children[0]);

            Instr jmp_to_end = make_instr(Opcode::JF, this->nextLabelName());
            this->instrs.push_back(jmp_to_end);

            this->generate(node->children[1]);
            Instr jmp_to_start = make_instr(Opcode::JMP, cond_label);
            this->instrs.push_back(jmp_to_start);
            this->labels[jmp_to_end.label] = this->instrs.size();
            break;
        }

        case NodeType::ADD_EXPR:
        case NodeType::SUB_EXPR:
        case NodeType::AND_EXPR:
        case NodeType::OR_EXPR:
        case NodeType::XOR_EXPR:
            this->generate(node->children[0]);
            this->generate(node->children[1]);
            this->instrs.push_back(make_instr(get_overloaded_op(node->type, node->datatype)));
            break;
        case NodeType::ASSIGN_EXPR: {
            Opcode set_op = this->symtab->isArgument(node->integer) ? Opcode::SETARG8 : this->symtab->isGlobal(node->integer) ? Opcode::SETGLOBAL8 : Opcode::SETLOCAL8;
            size_t stack_offset = this->symtab->getStackOffset(node->integer);
            this->generate(node->children[0]);
            this->instrs.push_back(make_instr(overload_size(node->datatype, Opcode::DUP8)));
            this->instrs.push_back(make_instr(overload_size(node->datatype, set_op), stack_offset));
            break;
        }
        case NodeType::ID_EXPR: {
            Opcode get_op = this->symtab->isArgument(node->integer) ? Opcode::GETARG8 : this->symtab->isGlobal(node->integer) ? Opcode::GETGLOBAL8 : Opcode::GETLOCAL8;
            size_t stack_offset = this->symtab->getStackOffset(node->integer);
            this->instrs.push_back(make_instr(overload_size(node->datatype, get_op), stack_offset));
            break;
        }
        case NodeType::CAST_EXPR: {
            DataType casted_type = node->datatype;
            DataType sub_type = node->children[0]->datatype;
            size_t new_size = datatype_size(casted_type);
            size_t old_size = datatype_size(sub_type);

            this->generate(node->children[0]);

            if(old_size > new_size) {
                size_t delta = old_size - new_size;
                this->instrs.push_back(make_instr(Opcode::FREE, delta));
            }
            else if(old_size < new_size) {
                size_t delta = new_size - old_size;
                this->instrs.push_back(make_instr(Opcode::ALLOC, delta));
            }

            break;
        }
        case NodeType::SUBSCRIPT_CONST: {
            Opcode get_op = this->symtab->isArgument(node->integer) ? Opcode::GETARG8 : this->symtab->isGlobal(node->integer) ? Opcode::GETGLOBAL8 : Opcode::GETLOCAL8;
            size_t symb_id = node->integer;
            size_t index = node->integer2;
            size_t stack_offset = this->symtab->getStackOffset(symb_id);
            stack_offset += index * datatype_size(node->datatype);
            this->instrs.push_back(make_instr(overload_size(node->datatype, get_op), stack_offset));
            break;
        }
        case NodeType::SUBSCRIPT_INDR: {
            size_t symb_id = node->integer;
            Opcode get_op = this->symtab->isArgument(symb_id) ? Opcode::GETARGIND8 : this->symtab->isGlobal(symb_id) ? Opcode::GETGLOBALIND8 : Opcode::GETLOCALIND8;
            size_t max_idx = (this->symtab->getArraySize(symb_id) - 1) * datatype_size(node->datatype) + 1;
            size_t stack_offset = this->symtab->getStackOffset(symb_id);
            this->generate(node->children[0]);

            size_t data_size = datatype_size(node->datatype);
            if(data_size > 1) {
                size_t shift_offset = std::countr_zero(data_size);
                this->instrs.push_back(make_instr(Opcode::IDXSHFT, shift_offset));
            }
            this->instrs.push_back(make_instr(overload_size(node->datatype, get_op), stack_offset, max_idx));
            break;
        }
        case NodeType::ARRAY_ASSIGN_CONST: {
            size_t symb_id = node->integer;
            size_t index = node->integer2;
            Opcode set_op = this->symtab->isArgument(symb_id) ? Opcode::SETARG8 : this->symtab->isGlobal(symb_id) ? Opcode::SETGLOBAL8 : Opcode::SETLOCAL8;
            size_t stack_offset = this->symtab->getStackOffset(symb_id);
            stack_offset += index * datatype_size(node->datatype);
            this->generate(node->children[0]);
            this->instrs.push_back(make_instr(overload_size(node->datatype, Opcode::DUP8)));
            this->instrs.push_back(make_instr(overload_size(node->datatype, set_op), stack_offset));
            break;
        }
        case NodeType::ARRAY_ASSIGN_INDR: {
            size_t symb_id = node->integer;
            Opcode set_op = this->symtab->isArgument(symb_id) ? Opcode::SETARGIND8 : this->symtab->isGlobal(symb_id) ? Opcode::SETGLOBALIND8 : Opcode::SETLOCALIND8;
            size_t max_idx = (this->symtab->getArraySize(symb_id) - 1) * datatype_size(node->datatype) + 1;
            size_t stack_offset = this->symtab->getStackOffset(symb_id);
            this->generate(node->children[1]);
            this->instrs.push_back(make_instr(overload_size(node->datatype, Opcode::DUP8)));
            this->generate(node->children[0]);

            size_t data_size = datatype_size(node->datatype);
            if(data_size > 1) {
                size_t shift_offset = std::countr_zero(data_size);
                this->instrs.push_back(make_instr(Opcode::IDXSHFT, shift_offset));
            }
            this->instrs.push_back(make_instr(overload_size(node->datatype, set_op), stack_offset, max_idx));
            break;
        }

        case NodeType::INT_CONST:
        case NodeType::U8_INT_CONST:
        case NodeType::U16_INT_CONST:
        case NodeType::U32_INT_CONST:
            this->instrs.push_back(make_instr(overload_size(node->datatype, Opcode::PUSH8), node->integer));
            break;
    }
}

std::vector<Instr> AsmGenerator::run() {
    this->generateGlobal(this->root);
    Instr make_args = make_instr(Opcode::MAKEARGS, 0);
    Instr call_entry = make_instr(Opcode::CALL, "entry");
    Instr accept = make_instr(Opcode::ACCEPT);
    this->instrs.push_back(make_args);
    this->instrs.push_back(call_entry);
    this->instrs.push_back(accept);

    this->generateFunctions(this->root);

    //Link
    for(Instr& instr : this->instrs) {
        if(instr.label.size() > 0) {
            if(this->labels.count(instr.label) == 0) {
                std::stringstream ss;
                ss << "Linker error, failed to find symbol ";
                ss << instr.label;
                throw std::runtime_error(ss.str());
            }
            instr.integer = this->labels[instr.label];
        }
    }

    return this->instrs;
}