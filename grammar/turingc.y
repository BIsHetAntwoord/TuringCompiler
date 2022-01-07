%code requires{
#include "frontend/parser.hpp"
#include "frontend/ast.hpp"
#include "frontend/symtab.hpp"

#include <string>
}

%{
#include "turingc.parse.h"
#include "turingc.lex.h"

#include <iostream>
%}

%define api.pure
%define parse.error detailed
%param {void* scanner}
%parse-param {parse_info* parser}

%union {
    ASTNode* node;
    uint64_t integer;
    DataType datatype;
    std::string* str;
}

%destructor {delete $$;} <node> <str>

%type<node> func_decl_list func_decl
%type<node> statement_list statement compound_statement
%type<node> declare_statement declare_init_statement declare_init_substat
%type<node> expr expr_no_int assign_expr int_constant read_expr
%type<datatype> datatype decl_datatype

%token U8 "u8"
%token U16 "u16"
%token U32 "u32"
%token VOID "void"
%token IF "if"
%token ELSE "else"
%token WHILE "while"
%token FUNCTION "function"
%token<str> ID "identifier"
%token<integer> INT "integer"
%token<integer> INT_U8 "u8 integer"
%token<integer> INT_U16 "u16 integer"
%token<integer> INT_U32 "u32 integer"

%right '='
%left '&' '|' '^'
%left '+' '-'

%%

root
    : func_decl_list                                {parser->ast = $1;}
    ;

func_decl_list
    : func_decl_list func_decl                      {$$ = new ASTNode(NodeType::LIST, {$1, $2});}
    | func_decl_list declare_statement ';'          {$$ = new ASTNode(NodeType::LIST, {$1, new ASTNode(NodeType::GLOBAL_DECL, {$2})});}
    |                                               {$$ = new ASTNode(NodeType::EMPTY, {});}
    ;

func_decl
    : FUNCTION ID                                   {parser->symtab->enterFunction(*$2);}
     '(' ')' ':' datatype                           {parser->symtab->declareFunction(*$2, $7);}
     compound_statement                             {$$ = new ASTNode(NodeType::FUNC_DECL, {$9}, $7, *$2); delete $2; parser->symtab->exitFunction();}
    ;

statement_list
    : statement_list statement                      {$$ = new ASTNode(NodeType::LIST, {$1, $2});}
    |                                               {$$ = new ASTNode(NodeType::EMPTY, {});}
    ;

statement
    : expr ';'                                      {$$ = new ASTNode(NodeType::EXPR_STAT, {$1});}
    | compound_statement                            {$$ = $1;}
    | IF '(' expr ')' compound_statement            {$$ = new ASTNode(NodeType::IF_STAT, {$3, $5});}
    | IF '(' expr ')' compound_statement
        ELSE compound_statement                     {$$ = new ASTNode(NodeType::IF_ELSE_STAT, {$3, $5, $7});}
    | WHILE '(' expr ')' compound_statement         {$$ = new ASTNode(NodeType::WHILE_STAT, {$3, $5});}
    | declare_statement ';'                         {$$ = $1;}
    ;

compound_statement
    : '{'                                           {parser->symtab->enterScope();}
       statement_list
      '}'                                           {parser->symtab->exitScope(); $$ = $3;}
    ;

declare_statement
    : decl_datatype ID                              {
                                                        if(!parser->symtab->declareSymbol(*$2, $1)) {
                                                            make_error(scanner, parser, "Redeclaration of symbol ", *$2);
                                                            delete $2;
                                                            YYERROR;
                                                        } else {
                                                            $$ = new ASTNode(NodeType::EMPTY, {});
                                                            delete $2;
                                                        }
                                                    }
    | decl_datatype '[' INT ']' ID                  {
                                                        if(!parser->symtab->declareArray(*$5, $1, $3)) {
                                                            make_error(scanner, parser, "Redeclaration of symbol ", *$5);
                                                            delete $5;
                                                            YYERROR;
                                                        } else {
                                                            $$ = new ASTNode(NodeType::EMPTY, {});
                                                            delete $5;
                                                        }
                                                    }
    | declare_init_statement                        {$$ = new ASTNode(NodeType::EXPR_STAT, {$1});}
    ;

declare_init_statement
    : decl_datatype ID '=' declare_init_substat     {
                                                        if(!parser->symtab->declareSymbol(*$2, $1)) {
                                                            make_error(scanner, parser, "Redeclaration of symbol ", *$2);
                                                            delete $2;
                                                            YYERROR;
                                                        }
                                                        else {
                                                            size_t symbol = parser->symtab->resolveSymbol(*$2);
                                                            $$ = new ASTNode(NodeType::ASSIGN_EXPR, {$4}, parser->symtab->getType(symbol), symbol);
                                                            delete $2;
                                                        }
                                                    }
    ;

declare_init_substat
    : declare_init_statement                        {$$ = $1;}
    | expr                                          {$$ = $1;}
    ;

expr
    : int_constant                                  {$$ = $1;}
    | expr_no_int                                   {$$ = $1;}
    ;

expr_no_int
    : expr '+' expr                                 {$$ = new ASTNode(NodeType::ADD_EXPR, {$1, $3});}
    | expr '-' expr                                 {$$ = new ASTNode(NodeType::SUB_EXPR, {$1, $3});}
    | expr '&' expr                                 {$$ = new ASTNode(NodeType::AND_EXPR, {$1, $3});}
    | expr '|' expr                                 {$$ = new ASTNode(NodeType::OR_EXPR, {$1, $3});}
    | expr '^' expr                                 {$$ = new ASTNode(NodeType::XOR_EXPR, {$1, $3});}
    | assign_expr                                   {$$ = $1;}
    | '(' expr ')'                                  {$$ = $2;}
    | read_expr                                     {$$ = $1;}
    | decl_datatype '(' expr ')'                    {$$ = new ASTNode(NodeType::CAST_EXPR, {$3}, $1);}
    ;

assign_expr
    : ID '=' expr                                   {
                                                        size_t symbol = parser->symtab->resolveSymbol(*$1);
                                                        if(symbol == INVALID_SYMBOL) {
                                                            make_error(scanner, parser, "Use of undeclared symbol ", *$1);
                                                            delete $1;
                                                            YYERROR;
                                                        }
                                                        else if(parser->symtab->isArray(symbol)) {
                                                            make_error(scanner, parser, "Use of array in non-array expression ", *$1);
                                                            delete $1;
                                                            YYERROR;
                                                        }
                                                        else {
                                                            $$ = new ASTNode(NodeType::ASSIGN_EXPR, {$3}, parser->symtab->getType(symbol), symbol);
                                                            delete $1;
                                                        }
                                                    }
    | ID '[' INT ']' '=' expr                       {
                                                        size_t symbol = parser->symtab->resolveSymbol(*$1);
                                                        if(symbol == INVALID_SYMBOL) {
                                                            make_error(scanner, parser, "Use of undeclared symbol ", *$1);
                                                            delete $1;
                                                            YYERROR;
                                                        }
                                                        else if(!parser->symtab->isArray(symbol)) {
                                                            make_error(scanner, parser, "Attempt to subscript non-array ", *$1);
                                                            delete $1;
                                                            YYERROR;
                                                        }
                                                        else {
                                                            $$ = new ASTNode(NodeType::ARRAY_ASSIGN_CONST, {$6}, parser->symtab->getType(symbol), symbol, $3);
                                                            delete $1;
                                                        }
                                                    }
    | ID '[' expr_no_int ']' '=' expr               {
                                                        size_t symbol = parser->symtab->resolveSymbol(*$1);
                                                        if(symbol == INVALID_SYMBOL) {
                                                            make_error(scanner, parser, "Use of undeclared symbol ", *$1);
                                                            delete $1;
                                                            YYERROR;
                                                        }
                                                        else if(!parser->symtab->isArray(symbol)) {
                                                            make_error(scanner, parser, "Attempt to subscript non-array ", *$1);
                                                            delete $1;
                                                            YYERROR;
                                                        }
                                                        else {
                                                            $$ = new ASTNode(NodeType::ARRAY_ASSIGN_INDR, {$3, $6}, parser->symtab->getType(symbol), symbol);
                                                            delete $1;
                                                        }
                                                    }
    ;

read_expr
    : ID                                            {
                                                        size_t symbol = parser->symtab->resolveSymbol(*$1);
                                                        if(symbol == INVALID_SYMBOL) {
                                                            make_error(scanner, parser, "Use of undeclared symbol ", *$1);
                                                            delete $1;
                                                            YYERROR;
                                                        }
                                                        else if(parser->symtab->isArray(symbol)) {
                                                            make_error(scanner, parser, "Use of array in non-array expression ", *$1);
                                                            delete $1;
                                                            YYERROR;
                                                        }
                                                        else {
                                                            $$ = new ASTNode(NodeType::ID_EXPR, {}, parser->symtab->getType(symbol), symbol);
                                                            delete $1;
                                                        }
                                                    }
    | ID '[' INT ']'                                {
                                                        size_t symbol = parser->symtab->resolveSymbol(*$1);
                                                        if(symbol == INVALID_SYMBOL) {
                                                            make_error(scanner, parser, "Use of undeclared symbol ", *$1);
                                                            delete $1;
                                                            YYERROR;
                                                        }
                                                        else if(!parser->symtab->isArray(symbol)) {
                                                            make_error(scanner, parser, "Attempt to subscript non-array ", *$1);
                                                            delete $1;
                                                            YYERROR;
                                                        }
                                                        else {
                                                            $$ = new ASTNode(NodeType::SUBSCRIPT_CONST, {}, parser->symtab->getType(symbol), symbol, $3);
                                                            delete $1;
                                                        }
                                                    }
    | ID '[' expr_no_int ']'                        {
                                                        size_t symbol = parser->symtab->resolveSymbol(*$1);
                                                        if(symbol == INVALID_SYMBOL) {
                                                            make_error(scanner, parser, "Use of undeclared symbol ", *$1);
                                                            delete $1;
                                                            YYERROR;
                                                        }
                                                        else if(!parser->symtab->isArray(symbol)) {
                                                            make_error(scanner, parser, "Attempt to subscript non-array ", *$1);
                                                            delete $1;
                                                            YYERROR;
                                                        }
                                                        else {
                                                            $$ = new ASTNode(NodeType::SUBSCRIPT_INDR, {$3}, parser->symtab->getType(symbol), symbol);
                                                            delete $1;
                                                        }
                                                    }
    ;

int_constant
    : INT                                           {$$ = new ASTNode(NodeType::INT_CONST, $1);}
    | INT_U8                                        {$$ = new ASTNode(NodeType::U8_INT_CONST, $1);}
    | INT_U16                                       {$$ = new ASTNode(NodeType::U16_INT_CONST, $1);}
    | INT_U32                                       {$$ = new ASTNode(NodeType::U32_INT_CONST, $1);}
    ;

datatype
    : VOID                                          {$$ = DataType::VOID;}
    | decl_datatype                                 {$$ = $1;}
    ;

decl_datatype
    : U8                                            {$$ = DataType::U8;}
    | U16                                           {$$ = DataType::U16;}
    | U32                                           {$$ = DataType::U32;}
    ;

%%