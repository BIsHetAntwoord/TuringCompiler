#include "frontend/symtab.hpp"

std::string GLOBAL_SCOPE_NAME = "$global";

Symtab::Symtab() {
    this->current_function = GLOBAL_SCOPE_NAME;

    this->enterScope();
}

void Symtab::enterFunction(const std::string& name) {
    this->current_function = name;

    this->function_locals[name] = 0;

    this->enterScope();
}

void Symtab::exitFunction() {
    this->current_function = GLOBAL_SCOPE_NAME;

    this->exitScope();
}

void Symtab::enterScope() {
    this->scopes.push_back({});
}

void Symtab::exitScope() {
    this->scopes.pop_back();
}

void Symtab::declareFunction(const std::string& name, DataType type) {
    FuncInfo func_info;
    func_info.ret_type = type;
    this->func_decls[name] = func_info;
}

bool Symtab::declareSymbol(const std::string& name, DataType type, bool is_arg) {
    auto& current_scope = this->scopes.back();
    if(current_scope.count(name) > 0)
        return false;

    auto& used_map = is_arg ? this->function_args : this->function_locals;
    size_t& stack_offset = used_map[this->current_function];

    SymbolInfo symb_info;
    symb_info.type = type;
    symb_info.is_arg = is_arg;
    symb_info.stack_offset = stack_offset;
    symb_info.is_array = false;
    symb_info.is_global = this->scopes.size() == 1;
    symb_info.array_size = 0;

    stack_offset += datatype_size(type);

    size_t result = this->symb_decls.size();
    this->symb_decls.push_back(symb_info);

    current_scope[name] = result;

    return true;
}

bool Symtab::declareArray(const std::string& name, DataType type, size_t array_size, bool is_arg) {
    auto& current_scope = this->scopes.back();
    if(current_scope.count(name) > 0)
        return false;

    auto& used_map = is_arg ? this->function_args : this->function_locals;
    size_t& stack_offset = used_map[this->current_function];

    SymbolInfo symb_info;
    symb_info.type = type;
    symb_info.is_arg = is_arg;
    symb_info.stack_offset = stack_offset;
    symb_info.is_array = true;
    symb_info.is_global = this->scopes.size() == 1;
    symb_info.array_size = array_size;

    stack_offset += datatype_size(type) * array_size;

    size_t result = this->symb_decls.size();
    this->symb_decls.push_back(symb_info);

    current_scope[name] = result;

    return true;
}

DataType Symtab::getType(size_t symb_id) {
    return this->symb_decls[symb_id].type;
}

bool Symtab::isArgument(size_t symb_id) {
    return this->symb_decls[symb_id].is_arg;
}

bool Symtab::isArray(size_t symb_id) {
    return this->symb_decls[symb_id].is_array;
}

bool Symtab::isGlobal(size_t symb_id) {
    return this->symb_decls[symb_id].is_global;
}

size_t Symtab::getStackOffset(size_t symb_id) {
    return this->symb_decls[symb_id].stack_offset;
}

size_t Symtab::getArraySize(size_t symb_id) {
    return this->symb_decls[symb_id].array_size;
}

size_t Symtab::resolveSymbol(const std::string& name) {
    for(size_t i = this->scopes.size(); i > 0; --i) {
        auto& scope = this->scopes[i-1];
        if(scope.count(name) > 0)
            return scope[name];
    }
    return INVALID_SYMBOL;
}

size_t Symtab::getFunctionLocalSize(const std::string& name) {
    return this->function_locals[name];
}

size_t Symtab::getGlobalSpaceSize() {
    return this->getFunctionLocalSize(GLOBAL_SCOPE_NAME);
}