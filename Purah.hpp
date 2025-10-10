#pragma once
#include <iostream>
#include <stdint.h>
#include <fstream>
#include <string>
#include <memory>
#include <stack>
#include <map>

#include "Purah lives here/Lexer.hpp"
#include "Purah lives here/Parser.hpp"
#include "Purah lives here/Exceptions.hpp"
#include "Purah lives here/Memory.hpp"
#include "Purah lives here/Functions.hpp"
#include "Purah lives here/AdditionalServices.hpp"

using namespace purah;

namespace purah {

    class Purah {
    public:
        Purah() {}
        lxr::Lexer lexer{};
        void tokenize(const std::string& input) {
            try {
                lexer.tokenize(input);
            } catch (const excptn::LexerError& e) {
                throw e;
            }
        }
        void parse() {
            if(parser) return;
            try {
                parser = std::make_unique<prsr::Parser>(lexer.get_tokens());
                parser->operator()();
            } catch (const excptn::ParserError& e) {
                throw e;
            }
            for(auto& func : parser->func_vector) {
                functions[fnc::FunctionSignature{func}] = std::move(func);
            }
        }
        int64_t interpret() {
            if(functions.empty()) return -1;
            int64_t result{};
            try {
                if(!functions.count(fnc::FunctionSignature{"main","int",std::vector<std::string>{}}))
                    throw excptn::PurahError("There's no main\\int() function!");
                calling_stack.push(PurahSpace{
                    std::move(functions[fnc::FunctionSignature{"main","int",std::vector<std::string>{}}]),
                    this,false});
                nds::ASTPtr return_node = calling_stack.top()();
                result = get_cpp_from_ast<int64_t>(return_node);
            } catch (const excptn::PurahError& e) {
                throw e;
            } catch (const excptn::MemoryError& e) {
                throw e;
            }
            return result;
        }
        static tkn::TokenType string_to_token_type(const std::string& type) {
            if (type == "int")     return tkn::INTEGER;
            if (type == "float")   return tkn::FLOAT;
            if (type == "bool")    return tkn::BOOL;
            if (type == "string")  return tkn::STRING;
            throw std::invalid_argument("Unknown token type: " + type);
        }
        static std::string type_token_to_string(const tkn::TokenType& type) {
            if(!tkn::is_single_value_type(type))
                throw std::invalid_argument("Unsupported type for type_token_to_string");
            if (type == tkn::INTEGER) return "int";
            if (type == tkn::FLOAT)   return "float";
            if (type == tkn::BOOL)    return "bool";
            if (type == tkn::STRING)  return "string";
        }
        template <typename T>
        static T get_cpp_from_ast(nds::ASTPtr& node) {
            if constexpr (std::is_same_v<T, int64_t>) {
                if (node->nodeType() != nds::IntExprType)
                    throw excptn::PurahError("Type mismatch: Expected IntExprNode");
                nds::IntExprNode* int_node = static_cast<nds::IntExprNode*>(node.get());
                return int_node->value;
            } else if constexpr (std::is_same_v<T, double>) {
                if (node->nodeType() != nds::FloatExprType)
                    throw excptn::PurahError("Type mismatch: Expected FloatExprNode");
                nds::FloatExprNode* float_node = static_cast<nds::FloatExprNode*>(node.get());
                return float_node->value;
            } else if constexpr (std::is_same_v<T, bool>) {
                if (node->nodeType() != nds::BoolExprType)
                    throw excptn::PurahError("Type mismatch: Expected BoolExprNode");
                nds::BoolExprNode* bool_node = static_cast<nds::BoolExprNode*>(node.get());
                return bool_node->value;
            } else if constexpr (std::is_same_v<T, std::string>) {
                if (node->nodeType() != nds::StringExprType)
                    throw excptn::PurahError("Type mismatch: Expected StringExprNode");
                nds::StringExprNode* string_node = static_cast<nds::StringExprNode*>(node.get());
                return string_node->value;
            } else {
                throw std::invalid_argument("Unsupported type for get_cpp_from_ast");
            }
        }
        class PurahSpace {
        public:
            PurahSpace(std::vector<nds::ASTPtr>&& ast, Purah* _purah, bool stack_right = true) 
                : AST_vector{std::move(ast)}, purah{_purah}, has_stack_right{stack_right} {}
            PurahSpace(nds::ASTPtr&& ast, Purah* _purah, bool stack_right = true) 
                : purah{_purah}, has_stack_right{stack_right} {
                if(ast->nodeType() != nds::FunctionExprType)
                    throw excptn::PurahError("InterpreterError: Expected FunctionExpr node for PurahSpace");
                nds::FunctionExprNode* func_node = static_cast<nds::FunctionExprNode*>(ast.get());
                AST_vector = std::move(func_node->body);
            }
            void set_var(nds::ASTPtr new_var_node) {
                if(new_var_node->nodeType() != nds::NewVarNodeType) 
                    throw excptn::PurahError("InterpreterError: Expected NewVar node at set_var");
                // Boo!~ //
            }
            nds::ASTPtr operator()() {
                for(nds::ASTPtr& node : AST_vector) {
                    nds::ASTPtr interpreted_node;
                    try {
                        interpreted_node = interpret_node(node);
                    } catch (const excptn::PurahError& e) {
                        throw e;
                    } catch (const excptn::MemoryError& e) {
                        throw e;
                    }
                    if(interpreted_node->nodeType() == nds::ReturnExprType) {
                        nds::ReturnExprNode* return_node = static_cast<nds::ReturnExprNode*>(interpreted_node.get());
                        nds::ASTPtr returned_node = std::move(return_node->value);
                        // Boo!~ //
                        return std::move(returned_node);
                    }
                }
                return std::make_unique<nds::ASTNode>();
            }
            nds::ASTPtr interpret_node(nds::ASTPtr&);
            nds::ASTPtr interpret_return(nds::ASTPtr&);
            nds::ASTPtr interpret_identifier(nds::ASTPtr&);
            nds::ASTPtr interpret_new_var(nds::ASTPtr&);
            nds::ASTPtr interpret_COUT(nds::ASTPtr&);
            std::vector<nds::ASTPtr> AST_vector{};
            mmry::VariableStorage    variable_storage{purah->global_storage};
            const bool               has_stack_right{};
        private:
            Purah* purah{};
        };
    private:
        std::unordered_map<fnc::FunctionSignature,nds::ASTPtr> functions{};
        std::unique_ptr<prsr::Parser>                          parser{nullptr};
        mmry::GlobalValueStorage                               global_storage{};
        std::stack<PurahSpace>                                 calling_stack{};
        mmry::VariableStorage                                  variable_storage{global_storage};
    };

}

nds::ASTPtr Purah::PurahSpace::interpret_node(nds::ASTPtr& node) {
    nds::ASTPtr return_node{};
    switch(node->nodeType()) {
        case nds::IdentifierExprType:
            return_node = interpret_identifier(node);
            break;
        case nds::IntExprType:
        case nds::FloatExprType:
        case nds::BoolExprType: 
        case nds::StringExprType:
            return_node = std::move(node);
            break;
        case nds::ReturnExprType:
            return_node = interpret_return(node);
            break;
        case nds::NewVarNodeType:
            return_node = interpret_new_var(node);
            break;
        case nds::COUTExprType:
            return_node = interpret_COUT(node);
            break;
        default:
            throw excptn::PurahError("I can't handle this node yet");
            break;
    }
    return std::move(return_node);
}
nds::ASTPtr Purah::PurahSpace::interpret_return(nds::ASTPtr& node) {
    if(node->nodeType() != nds::ReturnExprType)
        throw excptn::PurahError("InterpreterError: Expected ReturnExpr node at interpret_identifier");
    nds::ReturnExprNode* return_node = static_cast<nds::ReturnExprNode*>(node.get());
    return std::make_unique<nds::ReturnExprNode>(std::move(interpret_node(return_node->value)));
}
nds::ASTPtr Purah::PurahSpace::interpret_identifier(nds::ASTPtr& node) {
    if(node->nodeType() != nds::IdentifierExprType)
        throw excptn::PurahError("InterpreterError: Expected Identifier node at interpret_identifier");
    nds::IdentifierExprNode* id_node = static_cast<nds::IdentifierExprNode*>(node.get());
    std::string var_name = id_node->name;
    mmry::MemoryCellPair& cell = variable_storage.get_var(var_name);
    switch(cell.first) {
        case tkn::INTEGER:
            return std::make_unique<nds::IntExprNode>(std::get<int64_t>(cell.second));
        case tkn::FLOAT:
            return std::make_unique<nds::FloatExprNode>(std::get<double>(cell.second));
        case tkn::BOOL:
            return std::make_unique<nds::BoolExprNode>(std::get<bool>(cell.second));
        case tkn::STRING:
            return std::make_unique<nds::StringExprNode>(std::get<std::string>(cell.second));
        default:
            throw excptn::PurahError("Unsupported variable type at interpret_identifier");
            break;
    }
}
nds::ASTPtr Purah::PurahSpace::interpret_new_var(nds::ASTPtr& node) {
    if(node->nodeType() != nds::NewVarNodeType)
        throw excptn::PurahError("InterpreterError: Expected NewVar node at interpret_new_var");
    nds::NewVarNode* newvar_node = static_cast<nds::NewVarNode*>(node.get());
    std::string var_name         = newvar_node->name;
    tkn::TokenType var_type      = string_to_token_type(newvar_node->type);
    uint64_t address{};
    nds::ASTPtr value_node = interpret_node(newvar_node->value);
    switch(var_type) {
        case tkn::INTEGER: {
            address = variable_storage.new_var(var_name, var_type, int64_t{0});
            mmry::MemoryCellPair& cell = purah->global_storage.get_cell(address);
            if(value_node->nodeType() != nds::IntExprType)
                throw excptn::PurahError("Type mismatch: Expected IntExprNode");
            cell.second = get_cpp_from_ast<int64_t>(value_node);
        } break;
        case tkn::FLOAT: {
            address = variable_storage.new_var(var_name, var_type, double{0.0});
            mmry::MemoryCellPair& cell = purah->global_storage.get_cell(address);
            if(value_node->nodeType() != nds::FloatExprType)
                throw excptn::PurahError("Type mismatch: Expected FloatExprNode");
            cell.second = get_cpp_from_ast<double>(value_node);
        } break;
        case tkn::BOOL: {
            address = variable_storage.new_var(var_name, var_type, bool{false});
            mmry::MemoryCellPair& cell = purah->global_storage.get_cell(address);
            if(value_node->nodeType() != nds::BoolExprType)
                throw excptn::PurahError("Type mismatch: Expected BoolExprNode");
            cell.second = get_cpp_from_ast<bool>(value_node);
        } break;
        case tkn::STRING: {
            address = variable_storage.new_var(var_name, var_type, std::string{""});
            mmry::MemoryCellPair& cell = purah->global_storage.get_cell(address);
            if(value_node->nodeType() != nds::StringExprType)
                throw excptn::PurahError("Type mismatch: Expected StringExprNode");
            cell.second = get_cpp_from_ast<std::string>(value_node);
        } break;
        default:
            throw excptn::PurahError("Unsupported variable type at interpret_new_var");
            break;
    }
    return std::make_unique<nds::ASTNode>();
}
nds::ASTPtr Purah::PurahSpace::interpret_COUT(nds::ASTPtr& node) {
    if(node->nodeType() != nds::COUTExprType)
        throw excptn::PurahError("InterpreterError: Expected COUT node at interpret_COUT");
    nds::COUTExprNode* cout_node = static_cast<nds::COUTExprNode*>(node.get());
    nds::ASTPtr value_node = interpret_node(cout_node->value);
    switch(value_node->nodeType()) {
        case nds::IntExprType: {
            nds::IntExprNode* int_node = 
                static_cast<nds::IntExprNode*>(value_node.get());
            std::cout << int_node->value;
        } break;
        case nds::FloatExprType: {
            nds::FloatExprNode* float_node = 
                static_cast<nds::FloatExprNode*>(value_node.get());
            std::cout << float_node->value;
        } break;
        case nds::BoolExprType: {
            nds::BoolExprNode* bool_node = 
                static_cast<nds::BoolExprNode*>(value_node.get());
            std::cout << (bool_node->value ? "true" : "false");
        } break;
        case nds::StringExprType: {
            nds::StringExprNode* string_node = 
                static_cast<nds::StringExprNode*>(value_node.get());
            std::cout << string_node->value;
        } break;
        default: {
            throw excptn::PurahError("Unsupported COUT argument type");
        } break;
    }
    return std::make_unique<nds::ASTNode>();
}