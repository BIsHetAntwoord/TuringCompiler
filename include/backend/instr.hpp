#ifndef _TURINGCOMPILER_BACKEND_INSTR_HPP
#define _TURINGCOMPILER_BACKEND_INSTR_HPP

#include <iosfwd>
#include <cstdint>
#include <string>

enum class Opcode {
    // Stack
    PUSH8,
    PUSH16,
    PUSH32,
    POP8,
    POP16,
    POP32,
    DUP8,
    DUP16,
    DUP32,
    SWAP8,
    SWAP16,
    SWAP32,

    // Local stack frame
    ENTER,
    ALLOC,
    FREE,
    GETLOCAL8,
    GETLOCAL16,
    GETLOCAL32,
    GETLOCALIND8,
    GETLOCALIND16,
    GETLOCALIND32,
    SETLOCAL8,
    SETLOCAL16,
    SETLOCAL32,
    SETLOCALIND8,
    SETLOCALIND16,
    SETLOCALIND32,
    GETARG8,
    GETARG16,
    GETARG32,
    GETARGIND8,
    GETARGIND16,
    GETARGIND32,
    SETARG8,
    SETARG16,
    SETARG32,
    SETARGIND8,
    SETARGIND16,
    SETARGIND32,
    MAKEARGS,

    //Global memory
    GETGLOBAL8,
    GETGLOBAL16,
    GETGLOBAL32,
    GETGLOBALIND8,
    GETGLOBALIND16,
    GETGLOBALIND32,
    SETGLOBAL8,
    SETGLOBAL16,
    SETGLOBAL32,
    SETGLOBALIND8,
    SETGLOBALIND16,
    SETGLOBALIND32,

    //Arithmetic
    ADD8,
    ADD16,
    ADD32,
    SUB8,
    SUB16,
    SUB32,
    AND8,
    AND16,
    AND32,
    OR8,
    OR16,
    OR32,
    XOR8,
    XOR16,
    XOR32,

    //Address support
    IDXSHFT,

    //Control transfer
    JMP,
    JF,
    JT,
    CALL,
    RET,

    //Function utilities
    SETRET8,
    SETRET16,
    SETRET32,

    //Turing machine control
    ACCEPT,
    REJECT
};

struct Instr {
    Opcode opcode;
    uint64_t integer;
    uint64_t integer2;
    std::string label;
};

const char* opcode_name(Opcode);
std::ostream& operator<<(std::ostream&, const Opcode&);
std::ostream& operator<<(std::ostream&, const Instr&);

Instr make_instr(Opcode);
Instr make_instr(Opcode, uint64_t);
Instr make_instr(Opcode, uint64_t, uint64_t);
Instr make_instr(Opcode, const std::string&);

#endif
