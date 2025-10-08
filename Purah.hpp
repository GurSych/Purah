#pragma once
#include <iostream>
#include <stdint.h>
#include <fstream>
#include <string>
#include <memory>

#include "Purah lives here/Lexer.hpp"
#include "Purah lives here/Parser.hpp"
#include "Purah lives here/Exceptions.hpp"
#include "Purah lives here/Memory.hpp"

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
                parser     = std::make_unique<prsr::Parser>(lexer.get_tokens());
                std::vector<nds::ASTPtr>&& ast_vector = parser->operator()();
                AST_vector = std::move(ast_vector);
            } catch (const excptn::ParserError& e) {
                throw e;
            }
        }
        int64_t interpret() {
            if(AST_vector.empty()) return -1;
            int64_t result{};
            for(nds::ASTPtr& node : AST_vector) {
                nds::ASTPtr interpreted_node;
                try {
                    interpreted_node = interpret_node(node);
                } catch (const excptn::PurahError& e) {
                    throw e;
                } catch (const excptn::MemoryError& e) {
                    throw e;
                }
            }
            return result;
        }
        nds::ASTPtr interpret_node(nds::ASTPtr& node) {
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
                case nds::NewVarNodeType:
                    return_node = interpret_new_var(node);
                    break;
                case nds::COUTExprType:
                    return_node = interpret_COUT(node);
                    break;
                default:
                    throw excptn::PurahError("|| Purah: I can't handle this node yet");
                    break;
            }
            return std::move(return_node);
        }
        size_t AST_nodes_count() {
            return AST_vector.size();
        }
        static tkn::TokenType string_to_token_type(const std::string& type) {
            if (type == "int")     return tkn::INTEGER;
            if (type == "float")   return tkn::FLOAT;
            if (type == "bool")    return tkn::BOOL;
            if (type == "string")  return tkn::STRING;
            throw std::invalid_argument("Unknown token type: " + type);
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
    private:
        std::unique_ptr<prsr::Parser> parser{nullptr};
        std::vector<nds::ASTPtr>      AST_vector{};
        mmry::GlobalValueStorage      global_storage{};
        mmry::VariableStorage         variable_storage{global_storage};
        nds::ASTPtr interpret_identifier(nds::ASTPtr& node) {
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
        nds::ASTPtr interpret_new_var(nds::ASTPtr& node) {
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
                    mmry::MemoryCellPair& cell = global_storage.get_cell(address);
                    if(value_node->nodeType() != nds::IntExprType)
                        throw excptn::PurahError("Type mismatch: Expected IntExprNode");
                    cell.second = get_cpp_from_ast<int64_t>(value_node);
                } break;
                case tkn::FLOAT: {
                    address = variable_storage.new_var(var_name, var_type, double{0.0});
                    mmry::MemoryCellPair& cell = global_storage.get_cell(address);
                    if(value_node->nodeType() != nds::FloatExprType)
                        throw excptn::PurahError("Type mismatch: Expected FloatExprNode");
                    cell.second = get_cpp_from_ast<double>(value_node);
                } break;
                case tkn::BOOL: {
                    address = variable_storage.new_var(var_name, var_type, bool{false});
                    mmry::MemoryCellPair& cell = global_storage.get_cell(address);
                    if(value_node->nodeType() != nds::BoolExprType)
                        throw excptn::PurahError("Type mismatch: Expected BoolExprNode");
                    cell.second = get_cpp_from_ast<bool>(value_node);
                } break;
                case tkn::STRING: {
                    address = variable_storage.new_var(var_name, var_type, std::string{""});
                    mmry::MemoryCellPair& cell = global_storage.get_cell(address);
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
        nds::ASTPtr interpret_COUT(nds::ASTPtr& node) {
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
    };

}