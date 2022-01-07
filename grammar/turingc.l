%{
#include "frontend/parser.hpp"
#include "turingc.parse.h"
%}

%option reentrant
%option bison-bridge
%option noyywrap
%option nounput
%option never-interactive

%%

<INITIAL>{
    "u8"                                        {return U8;}
    "u16"                                       {return U16;}
    "u32"                                       {return U32;}
    "void"                                      {return VOID;}
    "if"                                        {return IF;}
    "else"                                      {return ELSE;}
    "while"                                     {return WHILE;}
    "function"                                  {return FUNCTION;}
    [a-zA-Z_][a-zA-Z0-9_]*                      {yylval->str = new std::string(yytext); return ID;}
    [0-9]+"u8"                                  {yylval->integer = strtoull(yytext, NULL, 10); return INT_U8;}
    [0-9]+"u16"                                 {yylval->integer = strtoull(yytext, NULL, 10); return INT_U16;}
    [0-9]+"u32"                                 {yylval->integer = strtoull(yytext, NULL, 10); return INT_U32;}
    [0-9]+                                      {yylval->integer = strtoull(yytext, NULL, 10); return INT;}
    [ \t\r\n]                                   {}
    .                                           {return yytext[0];}
}

%%
