#include "backend/instr.hpp"

#include <iostream>

const char* OPCODE_NAMES[] = {
    "PUSH8",
    "PUSH16",
    "PUSH32",
    "POP8",
    "POP16",
    "POP32",
    "DUP8",
    "DUP16",
    "DUP32",
    "SWAP8",
    "SWAP16",
    "SWAP32",
    "ENTER",
    "ALLOC",
    "FREE",
    "GETLOCAL8",
    "GETLOCAL16",
    "GETLOCAL32",
    "GETLOCALIND8",
    "GETLOCALIND16",
    "GETLOCALIND32",
    "SETLOCAL8",
    "SETLOCAL16",
    "SETLOCAL32",
    "SETLOCALIND8",
    "SETLOCALIND16",
    "SETLOCALIND32",
    "GETARG8",
    "GETARG16",
    "GETARG32",
    "GETARGIND8",
    "GETARGIND16",
    "GETARGIND32",
    "SETARG8",
    "SETARG16",
    "SETARG32",
    "SETARGIND8",
    "SETARGIND16",
    "SETARGIND32",
    "MAKEARGS",
    "GETGLOBAL8",
    "GETGLOBAL16",
    "GETGLOBAL32",
    "GETGLOBALIND8",
    "GETGLOBALIND16",
    "GETGLOBALIND32",
    "SETGLOBAL8",
    "SETGLOBAL16",
    "SETGLOBAL32",
    "SETGLOBALIND8",
    "SETGLOBALIND16",
    "SETGLOBALIND32",
    "ADD8",
    "ADD16",
    "ADD32",
    "SUB8",
    "SUB16",
    "SUB32",
    "AND8",
    "AND16",
    "AND32",
    "OR8",
    "OR16",
    "OR32",
    "XOR8",
    "XOR16",
    "XOR32",
    "IDXSHFT",
    "JMP",
    "JF",
    "JT",
    "CALL",
    "RET",
    "SETRET8",
    "SETRET16",
    "SETRET32",
    "ACCEPT",
    "REJECT"
};

const char* opcode_name(Opcode op) {
    return OPCODE_NAMES[static_cast<size_t>(op)];
}

std::ostream& operator<<(std::ostream& os, const Opcode& op) {
    os << opcode_name(op);
    return os;
}

std::ostream& operator<<(std::ostream& os, const Instr& instr) {
    os << instr.opcode;

    switch(instr.opcode) {
        case Opcode::PUSH8:
        case Opcode::PUSH16:
        case Opcode::PUSH32:
        case Opcode::SWAP8:
        case Opcode::SWAP16:
        case Opcode::SWAP32:
        case Opcode::ALLOC:
        case Opcode::FREE:
        case Opcode::GETLOCAL8:
        case Opcode::GETLOCAL16:
        case Opcode::GETLOCAL32:
        case Opcode::SETLOCAL8:
        case Opcode::SETLOCAL16:
        case Opcode::SETLOCAL32:
        case Opcode::GETARG8:
        case Opcode::GETARG16:
        case Opcode::GETARG32:
        case Opcode::SETARG8:
        case Opcode::SETARG16:
        case Opcode::SETARG32:
        case Opcode::MAKEARGS:
        case Opcode::GETGLOBAL8:
        case Opcode::GETGLOBAL16:
        case Opcode::GETGLOBAL32:
        case Opcode::SETGLOBAL8:
        case Opcode::SETGLOBAL16:
        case Opcode::SETGLOBAL32:
        case Opcode::IDXSHFT:
            os << " " << instr.integer;
            break;
        case Opcode::GETLOCALIND8:
        case Opcode::GETLOCALIND16:
        case Opcode::GETLOCALIND32:
        case Opcode::SETLOCALIND8:
        case Opcode::SETLOCALIND16:
        case Opcode::SETLOCALIND32:
        case Opcode::GETARGIND8:
        case Opcode::GETARGIND16:
        case Opcode::GETARGIND32:
        case Opcode::SETARGIND8:
        case Opcode::SETARGIND16:
        case Opcode::SETARGIND32:
        case Opcode::GETGLOBALIND8:
        case Opcode::GETGLOBALIND16:
        case Opcode::GETGLOBALIND32:
        case Opcode::SETGLOBALIND8:
        case Opcode::SETGLOBALIND16:
        case Opcode::SETGLOBALIND32:
            os << " " << instr.integer << ", " << instr.integer2;
            break;
        case Opcode::JMP:
        case Opcode::JF:
        case Opcode::JT:
        case Opcode::CALL:
            os << " " << instr.integer;
            if(instr.label.size() > 0)
                os << " (" << instr.label << ")";
            break;
        default:
            break;
    }
    return os;
}

Instr make_instr(Opcode op) {
    Instr instr;
    instr.opcode = op;
    return instr;
}

Instr make_instr(Opcode op, uint64_t integer) {
    Instr instr;
    instr.opcode = op;
    instr.integer = integer;
    return instr;
}

Instr make_instr(Opcode op, uint64_t integer, uint64_t integer2) {
    Instr instr;
    instr.opcode = op;
    instr.integer = integer;
    instr.integer2 = integer2;
    return instr;
}

Instr make_instr(Opcode op, const std::string& str) {
    Instr instr;
    instr.opcode = op;
    instr.integer = 0;
    instr.label = str;
    return instr;
}