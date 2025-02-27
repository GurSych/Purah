#pragma once
#include <variant>
#include <string>
#include <map>

#ifdef EOF
    #undef EOF
#endif

#include "Exceptions.hpp"
#include "Tokens.hpp"
#include "Lexer.hpp"

using Variant = std::variant<long long int,double,std::string,bool>;
using MemoryCell = std::pair<TokenType,std::variant<long long int,double,std::string,bool>>;

class Variable {
    public:
        Variable() {}
        Variable(TokenType _t, Variant _v) : type{_t}, value{_v} {}
        TokenType type{};
        Variant value{};
};

class VariableStorage {
    public:
        VariableStorage() {}
        void set_var(const std::string& name, Variable value) {
            variables[name] = value;
        }
        Variable get_var(const std::string& name) {
            if(variables.count(name)) return variables[name];
            throw MemoryError(std::string{"Undefined variable name: "}+name);
        }
    private:
        std::map<std::string,Variable> variables{};
};