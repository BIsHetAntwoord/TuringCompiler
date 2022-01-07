#ifndef _TURINGCOMPILER_ASSEMBLER_PARSER_HPP
#define _TURINGCOMPILER_ASSEMBLER_PARSER_HPP

#include <iosfwd>
#include <vector>
#include <string>
#include <unordered_map>

#include "backend/instr.hpp"
#include "exceptions.hpp"

class AssemblyParser {
    private:
        std::istream& input;
        std::unordered_map<std::string, size_t> labels;

        std::string removeComment(const std::string&);
        void parseLine(const std::string&, std::vector<Instr>&);

        template <typename T>
        T parseInteger(const std::string& str);
    public:
        AssemblyParser(std::istream&);

        std::vector<Instr> parse();
};

template <typename T>
T AssemblyParser::parseInteger(const std::string& str)  {
    if(str.size() == 0)
        throw ParseException("Integer required, empty string received");

    //TODO: bound check on type of T
    size_t offset = 0;
    size_t base = 10;
    if(str.size() > 2) {
        if(str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
            offset = 2;
            base = 16;
        }
        else if(str[0] == '0' && (str[1] == 'b' || str[1] == 'B')) {
            offset = 2;
            base = 2;
        }
    }
    else if(str.size() > 1) {
        if(str[0] == '0') {
            offset = 1;
            base = 8;
        }
    }

    if(offset > str.size())
        throw ParseException("Invalid integer literal");

    T result = T(0);
    for(size_t i = offset; i < str.size(); ++i) {
        size_t digit;
        if(str[i] >= '0' && str[i] <= '9')
            digit = str[i] - '0';
        else if(str[i] >= 'A' && str[i] <= 'Z')
            digit = str[i] - 'A' + 10;
        else if(str[i] >= 'a' && str[i] <= 'z')
            digit = str[i] - 'a' + 10;
        else
            throw ParseException("Unknown digit ", str[i], " in integer");
        if(digit >= base)
            throw ParseException("Invalid digit ", str[i], " in integer with base ", base);
        result *= base;
        result += digit;
    }
    return result;
}

#endif
