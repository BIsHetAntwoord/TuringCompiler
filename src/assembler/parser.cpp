#include "assembler/parser.hpp"
#include "exceptions.hpp"
#include "utils.hpp"

#include <iostream>
#include <string>
#include <cctype>
#include <unordered_map>

enum class OperandType {
    NONE,
    CONST8,
    CONST16,
    CONST32,
    CONST32_2,
    LABEL
};

const std::unordered_map<std::string, Opcode> OPCODE_MAP = {
    {"PUSH8", Opcode::PUSH8},
    {"PUSH16", Opcode::PUSH16},
    {"PUSH32", Opcode::PUSH32},
    {"POP8", Opcode::POP8},
    {"POP16", Opcode::POP16},
    {"POP32", Opcode::POP32},
    {"DUP8", Opcode::DUP8},
    {"DUP16", Opcode::DUP16},
    {"DUP32", Opcode::DUP32},
    {"SWAP8", Opcode::SWAP8},
    {"SWAP16", Opcode::SWAP16},
    {"SWAP32", Opcode::SWAP32},
    {"ENTER", Opcode::ENTER},
    {"ALLOC", Opcode::ALLOC},
    {"FREE", Opcode::FREE},
    {"GETLOCAL8", Opcode::GETLOCAL8},
    {"GETLOCAL16", Opcode::GETLOCAL16},
    {"GETLOCAL32", Opcode::GETLOCAL32},
    {"GETLOCALIND8", Opcode::GETLOCALIND8},
    {"GETLOCALIND16", Opcode::GETLOCALIND16},
    {"GETLOCALIND32", Opcode::GETLOCALIND32},
    {"SETLOCAL8", Opcode::SETLOCAL8},
    {"SETLOCAL16", Opcode::SETLOCAL16},
    {"SETLOCAL32", Opcode::SETLOCAL32},
    {"SETLOCALIND8", Opcode::SETLOCALIND8},
    {"SETLOCALIND16", Opcode::SETLOCALIND16},
    {"SETLOCALIND32", Opcode::SETLOCALIND32},
    {"GETARG8", Opcode::GETARG8},
    {"GETARG16", Opcode::GETARG16},
    {"GETARG32", Opcode::GETARG32},
    {"GETARGIND8", Opcode::GETARGIND8},
    {"GETARGIND16", Opcode::GETARGIND16},
    {"GETARGIND32", Opcode::GETARGIND32},
    {"SETARG8", Opcode::SETARG8},
    {"SETARG16", Opcode::SETARG16},
    {"SETARG32", Opcode::SETARG32},
    {"SETARGIND8", Opcode::SETARGIND8},
    {"SETARGIND16", Opcode::SETARGIND16},
    {"SETARGIND32", Opcode::SETARGIND32},
    {"MAKEARGS", Opcode::MAKEARGS},
    {"GETGLOBAL8", Opcode::GETGLOBAL8},
    {"GETGLOBAL16", Opcode::GETGLOBAL16},
    {"GETGLOBAL32", Opcode::GETGLOBAL32},
    {"GETGLOBALIND8", Opcode::GETGLOBALIND8},
    {"GETGLOBALIND16", Opcode::GETGLOBALIND16},
    {"GETGLOBALIND32", Opcode::GETGLOBALIND32},
    {"SETGLOBAL8", Opcode::SETGLOBAL8},
    {"SETGLOBAL16", Opcode::SETGLOBAL16},
    {"SETGLOBAL32", Opcode::SETGLOBAL32},
    {"SETGLOBALIND8", Opcode::SETGLOBALIND8},
    {"SETGLOBALIND16", Opcode::SETGLOBALIND16},
    {"SETGLOBALIND32", Opcode::SETGLOBALIND32},
    {"ADD8", Opcode::ADD8},
    {"ADD16", Opcode::ADD16},
    {"ADD32", Opcode::ADD32},
    {"SUB8", Opcode::SUB8},
    {"SUB16", Opcode::SUB16},
    {"SUB32", Opcode::SUB32},
    {"AND8", Opcode::AND8},
    {"AND16", Opcode::AND16},
    {"AND32", Opcode::AND32},
    {"OR8", Opcode::OR8},
    {"OR16", Opcode::OR16},
    {"OR32", Opcode::OR32},
    {"XOR8", Opcode::XOR8},
    {"XOR16", Opcode::XOR16},
    {"XOR32", Opcode::XOR32},
    {"IDXSHFT", Opcode::IDXSHFT},
    {"JMP", Opcode::JMP},
    {"JF", Opcode::JF},
    {"JT", Opcode::JT},
    {"CALL", Opcode::CALL},
    {"RET", Opcode::RET},
    {"SETRET8", Opcode::SETRET8},
    {"SETRET16", Opcode::SETRET16},
    {"SETRET32", Opcode::SETRET32},
    {"ACCEPT", Opcode::ACCEPT},
    {"REJECT", Opcode::REJECT}
};

const OperandType OPCODE_TYPES[] = {
    OperandType::CONST8, //PUSH8
    OperandType::CONST16, //PUSH16
    OperandType::CONST32, //PUSH32
    OperandType::NONE, //POP8
    OperandType::NONE, //POP16
    OperandType::NONE, //POP32
    OperandType::NONE, //DUP8
    OperandType::NONE, //DUP16
    OperandType::NONE, //DUP32
    OperandType::CONST32, //SWAP8
    OperandType::CONST32, //SWAP16
    OperandType::CONST32, //SWAP32
    OperandType::NONE, //ENTER
    OperandType::CONST32, //ALLOC
    OperandType::CONST32, //FREE
    OperandType::CONST32, //GETLOCAL8
    OperandType::CONST32, //GETLOCAL16
    OperandType::CONST32, //GETLOCAL32
    OperandType::CONST32_2, //GETLOCALIND8
    OperandType::CONST32_2, //GETLOCALIND16
    OperandType::CONST32_2, //GETLOCALIND32
    OperandType::CONST32, //SETLOCAL8
    OperandType::CONST32, //SETLOCAL16
    OperandType::CONST32, //SETLOCAL32
    OperandType::CONST32_2, //SETLOCALIND8
    OperandType::CONST32_2, //SETLOCALIND16
    OperandType::CONST32_2, //SETLOCALIND32
    OperandType::CONST32, //GETARG8
    OperandType::CONST32, //GETARG16
    OperandType::CONST32, //GETARG32
    OperandType::CONST32_2, //GETARGIND8
    OperandType::CONST32_2, //GETARGIND16
    OperandType::CONST32_2, //GETARGIND32
    OperandType::CONST32, //SETARG8
    OperandType::CONST32, //SETARG16
    OperandType::CONST32, //SETARG32
    OperandType::CONST32_2, //SETARGIND8
    OperandType::CONST32_2, //SETARGIND16
    OperandType::CONST32_2, //SETARGIND32
    OperandType::CONST32, //MAKEARGS
    OperandType::CONST32, //GETGLOBAL8
    OperandType::CONST32, //GETGLOBAL16
    OperandType::CONST32, //GETGLOBAL32
    OperandType::CONST32_2, //GETGLOBALIND8
    OperandType::CONST32_2, //GETGLOBALIND16
    OperandType::CONST32_2, //GETGLOBALIND32
    OperandType::CONST32, //SETGLOBAL8
    OperandType::CONST32, //SETGLOBAL16
    OperandType::CONST32, //SETGLOBAL32
    OperandType::CONST32_2, //SETGLOBALIND8
    OperandType::CONST32_2, //SETGLOBALIND16
    OperandType::CONST32_2, //SETGLOBALIND32
    OperandType::NONE, //ADD8
    OperandType::NONE, //ADD16
    OperandType::NONE, //ADD32
    OperandType::NONE, //SUB8
    OperandType::NONE, //SUB16
    OperandType::NONE, //SUB32
    OperandType::NONE, //AND8
    OperandType::NONE, //AND16
    OperandType::NONE, //AND32
    OperandType::NONE, //OR8
    OperandType::NONE, //OR16
    OperandType::NONE, //OR32
    OperandType::NONE, //XOR8
    OperandType::NONE, //XOR16
    OperandType::NONE, //XOR32
    OperandType::CONST32, //IDXSHFT
    OperandType::LABEL, //JMP
    OperandType::LABEL, //JF
    OperandType::LABEL, //JT
    OperandType::LABEL, //CALL
    OperandType::NONE, //RET
    OperandType::NONE, //SETRET8
    OperandType::NONE, //SETRET16
    OperandType::NONE, //SETRET32
    OperandType::NONE, //ACCEPT
    OperandType::NONE //REJECT
};

AssemblyParser::AssemblyParser(std::istream& input) : input(input) {}

std::string AssemblyParser::removeComment(const std::string& line) {
    auto it = line.find("#");
    if(it == std::string::npos)
        return line;
    return line.substr(0, it);
}

void AssemblyParser::parseLine(const std::string& line, std::vector<Instr>& result) {
    std::string stripped = utils_trim(this->removeComment(line));

    if(stripped.size() == 0)
        return;

    auto opcode_split = stripped.find_first_of(" \t\r\n");
    std::string opcode = stripped.substr(0, opcode_split);

    if(opcode[opcode.size() - 1] == ':') {
        std::string label_name = utils_trim(opcode.substr(0, opcode.size() - 1));
        this->labels[label_name] = result.size();
        return;
    }

    std::transform(opcode.begin(), opcode.end(), opcode.begin(), ::toupper);

    if(OPCODE_MAP.count(opcode) == 0)
        throw ParseException("Unknown opcode ", opcode);

    Opcode op = OPCODE_MAP.at(opcode);
    OperandType op_type = OPCODE_TYPES[static_cast<size_t>(op)];

    std::string operands_full = stripped.substr(opcode_split == std::string::npos ? stripped.size() : opcode_split);
    std::vector<std::string> operands = utils_split(operands_full, ',');
    for(std::string& op : operands)
        op = utils_trim(op);

    if(operands.size() == 1 && operands[0].size() == 0)
        operands.clear();

    Instr instr;
    instr.opcode = op;

    switch(op_type) {
        case OperandType::NONE:
            if(operands.size() > 0)
                throw ParseException("Operands given to opcode ", opcode, " without operands");
            break;
        case OperandType::CONST8:
            if(operands.size() != 1)
                throw ParseException("Wrong number of operands given to opcode ", opcode, ": ", operands.size(), " given, expected 1");
            instr.integer = this->parseInteger<uint8_t>(operands[0]);
            break;
        case OperandType::CONST16:
            if(operands.size() != 1)
                throw ParseException("Wrong number of operands given to opcode ", opcode, ": ", operands.size(), " given, expected 1");
            instr.integer = this->parseInteger<uint16_t>(operands[0]);
            break;
        case OperandType::CONST32:
            if(operands.size() != 1)
                throw ParseException("Wrong number of operands given to opcode ", opcode, ": ", operands.size(), " given, expected 1");
            instr.integer = this->parseInteger<uint32_t>(operands[0]);
            break;
        case OperandType::CONST32_2:
            if(operands.size() != 2)
                throw ParseException("Wrong number of operands given to opcode ", opcode, ": ", operands.size(), " given, expected 2");
            instr.integer = this->parseInteger<uint32_t>(operands[0]);
            instr.integer2 = this->parseInteger<uint32_t>(operands[1]);
            break;
        case OperandType::LABEL:
            if(operands.size() != 1)
                throw ParseException("Wrong number of operands given to opcode ", opcode, ": ", operands.size(), " given, expected 1");
            instr.label = operands[0];
            break;
    }

    result.push_back(instr);
}

std::vector<Instr> AssemblyParser::parse() {
    std::vector<Instr> result;

    std::string line;
    while(std::getline(this->input, line)) {
        this->parseLine(line, result);
    }

    for(Instr& instr : result) {
        if(instr.label.size() > 0) {
            if(this->labels.count(instr.label) == 0)
                throw ParseException("Reference to unknown label ", instr.label);
            instr.integer = this->labels[instr.label];
        }
    }

    return result;
}