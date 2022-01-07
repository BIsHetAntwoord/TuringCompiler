#ifndef _TURINGCOMPILER_FRONTEND_SYMTAB_HPP
#define _TURINGCOMPILER_FRONTEND_SYMTAB_HPP

#include "frontend/ast.hpp"

#include <vector>
#include <unordered_map>
#include <string>
#include <cstddef>
#include <limits>

const size_t INVALID_SYMBOL = std::numeric_limits<size_t>::max();

struct SymbolInfo {
    DataType type;
    bool is_arg;
    bool is_global;
    bool is_array;
    size_t array_size;
    size_t stack_offset;
};

struct FuncInfo {
    DataType ret_type;
    std::vector<DataType> args;
};

class Symtab {
    private:
        std::vector<SymbolInfo> symb_decls;
        std::vector<std::unordered_map<std::string, size_t>> scopes;
        std::unordered_map<std::string, size_t> function_locals;
        std::unordered_map<std::string, size_t> function_args;
        std::unordered_map<std::string, FuncInfo> func_decls;

        std::string current_function;
    public:
        Symtab();

        void enterFunction(const std::string&);
        void exitFunction();

        void enterScope();
        void exitScope();

        void declareFunction(const std::string&, DataType);
        bool declareSymbol(const std::string&, DataType, bool = false);
        bool declareArray(const std::string&, DataType, size_t, bool = false);

        size_t resolveSymbol(const std::string&);
        DataType getType(size_t);
        bool isArgument(size_t);
        bool isArray(size_t);
        bool isGlobal(size_t);
        size_t getStackOffset(size_t);
        size_t getArraySize(size_t);
        size_t getFunctionLocalSize(const std::string&);
        size_t getGlobalSpaceSize();
};

#endif
