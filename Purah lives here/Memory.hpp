#pragma once
#include <stdint.h>
#include <variant>
#include <string>
#include <queue>
#include <map>

#include <iostream>

#ifdef EOF
    #undef EOF
#endif

#include "Exceptions.hpp"
#include "Tokens.hpp"
#include "Lexer.hpp"

using namespace purah;

namespace purah { namespace mmry {

    using TypeVariant = std::variant<int64_t,double,std::string,bool>;
    using MemoryCellPair = std::pair<tkn::TokenType,TypeVariant>;

    class GlobalValueStorage {
        public:
            GlobalValueStorage() { }
            template<typename T>
            uint64_t new_cell(tkn::TokenType type, T value) {
                uint64_t address{};
                if(!free_addresses.empty()) {
                    address = free_addresses.top();
                    free_addresses.pop();
                } else {
                    address = max_address++;
                }
                memory[address] = MemoryCellPair{type,value};
                return address;
            }
            MemoryCellPair& get_cell(const uint64_t address) {
                if(!memory.count(address))
                    throw excptn::MemoryError(std::string{"Undefined memory address: "}+std::to_string(address));
                return memory[address];
            }
            void delete_cell(const uint64_t address) {
                if(memory.count(address)) {
                    memory.erase(address);
                    free_addresses.push(address);
                }
            }
        private:
            std::unordered_map<uint64_t,MemoryCellPair> memory{};
            std::priority_queue<uint64_t, std::vector<uint64_t>, std::greater<uint64_t>> free_addresses{};
            uint64_t max_address{};
    };

    class VariableStorage {
        public:
            VariableStorage(GlobalValueStorage& _global_storage) : global_storage{_global_storage} { }
            template<typename T>
            uint64_t new_var(const std::string& name, tkn::TokenType type, T value) {
                if(variables.count(name)) 
                    throw excptn::MemoryError(std::string{"Redefinition of variable name: "}+name);
                uint64_t address = global_storage.new_cell(type, value);
                variables[name] = address;
                return address;
            }
            template<typename T>
            MemoryCellPair& set_var(const std::string& name, T value) {
                if(!variables.count(name))
                    throw excptn::MemoryError(std::string{"Undefined variable name: "}+name);
                uint64_t address = variables[name];
                MemoryCellPair& cell = global_storage.get_cell(address);
                cell.second = value;
                return cell;
            }
            MemoryCellPair& get_var(const std::string& name) {
                if(variables.count(name)) return global_storage.get_cell(variables[name]);
                throw excptn::MemoryError(std::string{"Undefined variable name: "}+name);
            }
            ~VariableStorage() {
                for(const auto& variable : variables) {
                    global_storage.delete_cell(variable.second);
                }
            }
        private:
            GlobalValueStorage& global_storage;
            std::map<std::string,uint64_t> variables{};
    };

} }