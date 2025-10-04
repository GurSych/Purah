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

using namespace purah;

namespace purah { namespace mmry {

    using TypeVariant = std::variant<long long int,double,std::string,bool>;
    using MemoryCellPair = std::pair<tkn::TokenType,TypeVariant>;

    class Variable {
        public:
            Variable() {}
            Variable(tkn::TokenType _t, TypeVariant _v) : type{_t}, value{_v} {}
            tkn::TokenType type{};
            TypeVariant value{};
    };

    class VariableStorage {
        public:
            VariableStorage() {}
            void set_var(const std::string& name, Variable value) {
                variables[name] = value;
            }
            Variable get_var(const std::string& name) {
                if(variables.count(name)) return variables[name];
                throw excptn::MemoryError(std::string{"Undefined variable name: "}+name);
            }
        private:
            std::map<std::string,Variable> variables{};
    };

    class GlobalValueStorage {
        public:
            GlobalValueStorage() {}
            size_t new_cell() {
                return 0ull;
            }
            const MemoryCellPair& get_cell(const size_t address) {
                if(memory.count(address)) return memory[address];
                    throw excptn::MemoryError(std::string{"Undefined memory address: "}+std::to_string(address));
            }
        private:
            std::map<size_t,MemoryCellPair> memory{};
    };

} }