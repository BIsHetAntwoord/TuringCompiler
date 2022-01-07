#ifndef _TURINGCOMPILER_FRONTEND_PARSER_HPP
#define _TURINGCOMPILER_FRONTEND_PARSER_HPP

#include <cstdint>

#include "utils.hpp"

class ASTNode;
class Symtab;

struct parse_info {
    ASTNode* ast;
    Symtab* symtab;
};


void yyerror(void*, parse_info*, const char*);

template <typename... Args>
void make_error(void* scanner, parse_info* parser, const Args&... args) {
    std::string err_msg = utils_make_str(args...);
    yyerror(scanner, parser, err_msg.c_str());
}

#endif
