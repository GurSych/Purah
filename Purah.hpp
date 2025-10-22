#pragma once
#include <iostream>
#include <stdint.h>
#include <optional>
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
#include "Purah lives here/ParserNodes.hpp"

using namespace purah;

namespace purah {

    class Purah {
    public:
        Purah() {}
        lxr::Lexer lexer{};
        void tokenize(const std::string& input, size_t line = 0ull) {
            try {
                lexer.tokenize(input, line);
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
            for(nds::ASTPtr& func : parser->func_vector) {
                try {
                    fnc::FunctionSignature sign{func};
                    if(functions.contains(sign))
                        throw excptn::ParserError("Redefinition of function signature: " + sign.to_string(false));
                    functions[sign] = std::move(func);
                } catch (const excptn::PurahError& e) {
                    throw e;
                } catch (const excptn::ParserError& e) { 
                    throw e;
                }
            }
        }
        int64_t interpret() {
            if(functions.empty()) return -1;
            int64_t result{};
            fnc::FunctionSignature main_sign{"main","int",std::vector<std::string>{}};
            try {
                if(!functions.count(main_sign))
                    throw excptn::PurahError("There's no main\\int() function!");
                nds::ASTPtr main_func = std::move(functions[main_sign]);
                if(!main_sign.check_all(main_func))
                    throw excptn::PurahError("There's no main\\int() function!");
                calling_stack.emplace(main_func,this,nullptr);
                nds::ASTPtr return_node = calling_stack.top()();
                result = get_cpp_from_ast<int64_t>(return_node);
                calling_stack.pop();
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
        static std::string type_token_to_string(const tkn::TokenType type) {
            if (type == tkn::INTEGER) return "int";
            if (type == tkn::FLOAT)   return "float";
            if (type == tkn::BOOL)    return "bool";
            if (type == tkn::STRING)  return "string";
            throw std::invalid_argument("Unsupported type for type_token_to_string");
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
        template <typename T>
        static nds::ASTPtr get_ast_from_cpp(T value) {
            if constexpr (std::is_same_v<T, int64_t>) {
                return std::make_shared<nds::IntExprNode>(value);
            } else if constexpr (std::is_same_v<T, double>) {
                return std::make_shared<nds::FloatExprNode>(value);
            } else if constexpr (std::is_same_v<T, bool>) {
                return std::make_shared<nds::BoolExprNode>(value);
            } else if constexpr (std::is_same_v<T, std::string>) {
                return std::make_shared<nds::StringExprNode>(value);
            } else {
                throw std::invalid_argument("Unsupported type for get_ast_from_cpp");
            }
        }
        class PurahSpace {
        public:
            PurahSpace(std::vector<nds::ASTPtr> ast, Purah* _purah, PurahSpace* _p_s = nullptr) 
                : AST_vector{ast}, purah{_purah}, variable_storage{_purah->global_storage}, previous_space{_p_s} {}
            PurahSpace(nds::ASTPtr ast, Purah* _purah, PurahSpace* _p_s = nullptr) 
                : purah{_purah}, variable_storage{_purah->global_storage}, previous_space{_p_s} {
                if(ast->nodeType() == nds::FunctionExprType) {
                    nds::FunctionExprNode* func_node = static_cast<nds::FunctionExprNode*>(ast.get());
                    AST_vector = func_node->body;
                }
                else if(ast->nodeType() == nds::IfExprType) {
                    nds::IfExprNode* if_node = static_cast<nds::IfExprNode*>(ast.get());
                    AST_vector = if_node->body;
                }
                else
                    throw excptn::PurahError("InterpreterError: Expected FunctionExpr node for PurahSpace");
            }
            void set_var(nds::ASTPtr new_var_node) {
                if(new_var_node->nodeType() != nds::NewVarNodeType) 
                    throw excptn::PurahError("InterpreterError: Expected NewVar node at set_var");
                interpret_new_var(new_var_node);
            }
            std::optional<mmry::MemoryCellPair*> get_var(const std::string& name) {
                if(variable_storage.check_var(name)) {
                    return std::optional<mmry::MemoryCellPair*>{&variable_storage.get_var(name)};
                }
                if(previous_space == nullptr) {
                    return std::optional<mmry::MemoryCellPair*>{};
                }
                return previous_space->get_var(name);
            }
            nds::ASTPtr operator()() {
                for(nds::ASTPtr& node : AST_vector) {
                    nds::ASTPtr interpreted_node{};
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
                        return std::move(returned_node);
                    }
                }
                throw excptn::PurahError("Expected a return node");
            }
            nds::ASTPtr if_call() {
                for(nds::ASTPtr& node : AST_vector) {
                    nds::ASTPtr interpreted_node{};
                    try {
                        interpreted_node = interpret_node(node);
                    } catch (const excptn::PurahError& e) {
                        throw e;
                    } catch (const excptn::MemoryError& e) {
                        throw e;
                    }
                    if(interpreted_node->nodeType() == nds::ReturnExprType) {
                        return std::move(interpreted_node);
                    }
                }
                return std::make_shared<nds::ASTNode>();
            }
            nds::ASTPtr interpret_node(nds::ASTPtr&);
            nds::ASTPtr interpret_function_call(nds::ASTPtr&);
            nds::ASTPtr interpret_return(nds::ASTPtr&);
            nds::ASTPtr interpret_identifier(nds::ASTPtr&);
            nds::ASTPtr interpret_binary(nds::ASTPtr&);
            nds::ASTPtr interpret_new_var(nds::ASTPtr&);
            nds::ASTPtr interpret_if(nds::ASTPtr&);
            nds::ASTPtr interpret_COUT(nds::ASTPtr&);
            std::vector<nds::ASTPtr> AST_vector{};
            mmry::VariableStorage    variable_storage;
        private:
            Purah*      purah{};
            PurahSpace* previous_space{};
        };
    private:
        std::unordered_map<fnc::FunctionSignature,nds::ASTPtr> functions{};
        std::unique_ptr<prsr::Parser>                          parser{nullptr};
        mmry::GlobalValueStorage                               global_storage{};
        std::stack<PurahSpace>                                 calling_stack{};
        mmry::VariableStorage                                  variable_storage{global_storage};
    public:
        std::stack<PurahSpace>& get_stack() {
            return calling_stack;
        }
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
            return_node = node;
            break;
        case nds::BinaryExprType:
            return_node = interpret_binary(node);
            break;
        case nds::CallExprType:
            return_node = interpret_function_call(node);
            break;
        case nds::ReturnExprType:
            return_node = interpret_return(node);
            break;
        case nds::NewVarNodeType:
            return_node = interpret_new_var(node);
            break;
        case nds::IfExprType:
            return_node = interpret_if(node);
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
nds::ASTPtr Purah::PurahSpace::interpret_function_call(nds::ASTPtr& node) {
    if(node->nodeType() != nds::CallExprType)
        throw excptn::PurahError("InterpreterError: Expected CallExpr node at interpret_function_call");
    nds::CallExprNode* call_node = static_cast<nds::CallExprNode*>(node.get());
    std::vector<nds::ASTPtr> arguments{};
    for(nds::ASTPtr& arg : call_node->args) {
        arguments.push_back(std::move(interpret_node(arg)));
    }
    if(!call_node->authorized) {
        std::vector<std::string> args{};
        for(nds::ASTPtr& arg : arguments) {
            args.push_back(nds::type_token_to_string(arg->nodeType()));
        }
        fnc::FunctionSignature sign = fnc::FunctionSignature{call_node->name,args};
        if(purah->functions.contains(sign)) {
            call_node->function   = purah->functions[sign];
            call_node->authorized = true;
        }
        else throw excptn::PurahError("There's no function with " + sign.to_string(false) + " signature to call");
    }
    fnc::FunctionSignature func_sign{call_node->function};
    purah->calling_stack.emplace(call_node->function,purah);
    std::vector<nds::ASTPtr>::const_iterator arg = arguments.cbegin();
    std::vector<std::string>::const_iterator name_arg = func_sign.args_names.cbegin();
    for(auto type_arg = func_sign.args_types.cbegin(); type_arg < func_sign.args_types.cend(); ++type_arg, ++name_arg, ++arg) {
        if(arg == arguments.cend())
            throw excptn::PurahError("Missing argument for "+func_sign.to_string()+" signature calling");
        nds::ASTPtr new_var_node = std::make_shared<nds::NewVarNode>(*type_arg,*name_arg,*arg);
        purah->calling_stack.top().set_var(new_var_node);
    }
    nds::ASTPtr return_node = purah->calling_stack.top()();
    if(nds::type_token_to_string(return_node->nodeType()) != func_sign.type)
        throw excptn::PurahError("Incorrect returning type for "+func_sign.to_string()+" signature");
    purah->calling_stack.pop();
    return std::move(return_node);
}
nds::ASTPtr Purah::PurahSpace::interpret_return(nds::ASTPtr& node) {
    if(node->nodeType() != nds::ReturnExprType)
        throw excptn::PurahError("InterpreterError: Expected ReturnExpr node at interpret_return");
    nds::ReturnExprNode* return_node = static_cast<nds::ReturnExprNode*>(node.get());
    return std::make_shared<nds::ReturnExprNode>(std::move(interpret_node(return_node->value)));
}
nds::ASTPtr Purah::PurahSpace::interpret_identifier(nds::ASTPtr& node) {
    if(node->nodeType() != nds::IdentifierExprType)
        throw excptn::PurahError("InterpreterError: Expected Identifier node at interpret_identifier");
    nds::IdentifierExprNode* id_node = static_cast<nds::IdentifierExprNode*>(node.get());
    std::string var_name = id_node->name;
    std::optional<mmry::MemoryCellPair*> cell_optional = get_var(var_name);
    if(!cell_optional.has_value())
        throw excptn::PurahError(std::string{"Undefined variable name: "}+var_name);
    mmry::MemoryCellPair* cell = cell_optional.value();
    switch(cell->first) {
        case tkn::INTEGER:
            return std::make_shared<nds::IntExprNode>(std::get<int64_t>(cell->second));
        case tkn::FLOAT:
            return std::make_shared<nds::FloatExprNode>(std::get<double>(cell->second));
        case tkn::BOOL:
            return std::make_shared<nds::BoolExprNode>(std::get<bool>(cell->second));
        case tkn::STRING:
            return std::make_shared<nds::StringExprNode>(std::get<std::string>(cell->second));
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
    return std::make_shared<nds::ASTNode>();
}
nds::ASTPtr Purah::PurahSpace::interpret_if(nds::ASTPtr& node) {
    nds::ASTPtr local_node = node;
    while(true) {
        if(local_node->nodeType() == nds::SimpleAST) break;
        if(local_node->nodeType() != nds::IfExprType)
            throw excptn::PurahError("InterpreterError: Expected IfExpr node at interpret_if");
        nds::IfExprNode* if_node = static_cast<nds::IfExprNode*>(local_node.get());
        nds::ASTPtr condition    = std::move(interpret_node(if_node->condition));
        if(condition->nodeType() != nds::BoolExprType)
            throw excptn::PurahError("If-statement condition must be bool");
        nds::BoolExprNode* cond_node = static_cast<nds::BoolExprNode*>(condition.get());
        if(cond_node->value == true) {
            purah->calling_stack.emplace(local_node,purah,this);
            nds::ASTPtr return_node = purah->calling_stack.top().if_call();
            purah->calling_stack.pop();
            if(return_node->nodeType() == nds::ReturnExprType) {
                return std::move(return_node);
            }
            break;
        }
        else {
            local_node = if_node->next_if;
        }
    }
    return std::make_shared<nds::ASTNode>();
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
            throw excptn::PurahError("Unsupported COUT node type");
        } break;
    }
    return std::make_shared<nds::ASTNode>();
}
nds::ASTPtr Purah::PurahSpace::interpret_binary(nds::ASTPtr& node) {
    if(node->nodeType() != nds::BinaryExprType)
        throw excptn::PurahError("InterpreterError: Expected BinaryExpr node at interpret_binary");
    nds::BinaryExprNode* bin_node = static_cast<nds::BinaryExprNode*>(node.get());
    nds::ASTPtr left  = interpret_node(bin_node->left);
    nds::ASTPtr right = interpret_node(bin_node->right);
    switch(bin_node->op) {
        case tkn::PLUS: {
            if(left->nodeType() != right->nodeType())
                throw excptn::PurahError("Nodes should has the same type to be handeled by operator");
            switch(left->nodeType()) {
                case nds::IntExprType: {
                    int64_t left_value  = get_cpp_from_ast<int64_t>(left);
                    int64_t right_value = get_cpp_from_ast<int64_t>(right);
                    return get_ast_from_cpp<int64_t>(left_value+right_value);
                } break;
                case nds::FloatExprType: {
                    double left_value  = get_cpp_from_ast<double>(left);
                    double right_value = get_cpp_from_ast<double>(right);
                    return get_ast_from_cpp<double>(left_value+right_value);
                } break;
                case nds::StringExprType: {
                    std::string left_value  = get_cpp_from_ast<std::string>(left);
                    std::string right_value = get_cpp_from_ast<std::string>(right);
                    return get_ast_from_cpp<std::string>(left_value+right_value);
                } break;
                case nds::BoolExprType: {
                    bool left_value  = get_cpp_from_ast<bool>(left);
                    bool right_value = get_cpp_from_ast<bool>(right);
                    return get_ast_from_cpp<bool>(left_value+right_value);
                } break;
                default:
                    throw excptn::PurahError("Unsupported type for + operator");
                    break;
            }
        } break;
        case tkn::MINUS: {
            if(left->nodeType() != right->nodeType())
                throw excptn::PurahError("Nodes should has the same type to be handeled by operator");
            switch(left->nodeType()) {
                case nds::IntExprType: {
                    int64_t left_value  = get_cpp_from_ast<int64_t>(left);
                    int64_t right_value = get_cpp_from_ast<int64_t>(right);
                    return get_ast_from_cpp<int64_t>(left_value-right_value);
                } break;
                case nds::FloatExprType: {
                    double left_value  = get_cpp_from_ast<double>(left);
                    double right_value = get_cpp_from_ast<double>(right);
                    return get_ast_from_cpp<double>(left_value-right_value);
                } break;
                case nds::BoolExprType: {
                    bool left_value  = get_cpp_from_ast<bool>(left);
                    bool right_value = get_cpp_from_ast<bool>(right);
                    return get_ast_from_cpp<bool>(left_value-right_value);
                } break;
                default:
                    throw excptn::PurahError("Unsupported type for - operator");
                    break;
            }
        } break;
        case tkn::STAR: {
            if(left->nodeType() != right->nodeType())
                throw excptn::PurahError("Nodes should has the same type to be handeled by operator");
            switch(left->nodeType()) {
                case nds::IntExprType: {
                    int64_t left_value  = get_cpp_from_ast<int64_t>(left);
                    int64_t right_value = get_cpp_from_ast<int64_t>(right);
                    return get_ast_from_cpp<int64_t>(left_value*right_value);
                } break;
                case nds::FloatExprType: {
                    double left_value  = get_cpp_from_ast<double>(left);
                    double right_value = get_cpp_from_ast<double>(right);
                    return get_ast_from_cpp<double>(left_value*right_value);
                } break;
                case nds::BoolExprType: {
                    bool left_value  = get_cpp_from_ast<bool>(left);
                    bool right_value = get_cpp_from_ast<bool>(right);
                    return get_ast_from_cpp<bool>(left_value*right_value);
                } break;
                default:
                    throw excptn::PurahError("Unsupported type for * operator");
                    break;
            }
        } break;
        case tkn::SLASH: {
            if(left->nodeType() != right->nodeType())
                throw excptn::PurahError("Nodes should has the same type to be handeled by operator");
            switch(left->nodeType()) {
                case nds::IntExprType: {
                    int64_t left_value  = get_cpp_from_ast<int64_t>(left);
                    int64_t right_value = get_cpp_from_ast<int64_t>(right);
                    return get_ast_from_cpp<double>(left_value/right_value);
                } break;
                case nds::FloatExprType: {
                    double left_value  = get_cpp_from_ast<double>(left);
                    double right_value = get_cpp_from_ast<double>(right);
                    return get_ast_from_cpp<double>(left_value/right_value);
                } break;
                case nds::BoolExprType: {
                    bool left_value  = get_cpp_from_ast<bool>(left);
                    bool right_value = get_cpp_from_ast<bool>(right);
                    return get_ast_from_cpp<bool>(left_value/right_value);
                } break;
                default:
                    throw excptn::PurahError("Unsupported type for / operator");
                    break;
            }
        } break;
        case tkn::EQUALITY: {
            if(left->nodeType() != right->nodeType())
                throw excptn::PurahError("Nodes should has the same type to be handeled by operator");
            switch(left->nodeType()) {
                case nds::IntExprType: {
                    int64_t left_value  = get_cpp_from_ast<int64_t>(left);
                    int64_t right_value = get_cpp_from_ast<int64_t>(right);
                    return get_ast_from_cpp<bool>(left_value==right_value);
                } break;
                case nds::FloatExprType: {
                    double left_value  = get_cpp_from_ast<double>(left);
                    double right_value = get_cpp_from_ast<double>(right);
                    return get_ast_from_cpp<bool>(left_value==right_value);
                } break;
                case nds::StringExprType: {
                    std::string left_value  = get_cpp_from_ast<std::string>(left);
                    std::string right_value = get_cpp_from_ast<std::string>(right);
                    return get_ast_from_cpp<bool>(left_value==right_value);
                } break;
                case nds::BoolExprType: {
                    bool left_value  = get_cpp_from_ast<bool>(left);
                    bool right_value = get_cpp_from_ast<bool>(right);
                    return get_ast_from_cpp<bool>(left_value==right_value);
                } break;
                default:
                    throw excptn::PurahError("Unsupported type for == operator");
                    break;
            }
        } break;
        case tkn::MORE: {
            if(left->nodeType() != right->nodeType())
                throw excptn::PurahError("Nodes should has the same type to be handeled by operator");
            switch(left->nodeType()) {
                case nds::IntExprType: {
                    int64_t left_value  = get_cpp_from_ast<int64_t>(left);
                    int64_t right_value = get_cpp_from_ast<int64_t>(right);
                    return get_ast_from_cpp<bool>(left_value>right_value);
                } break;
                case nds::FloatExprType: {
                    double left_value  = get_cpp_from_ast<double>(left);
                    double right_value = get_cpp_from_ast<double>(right);
                    return get_ast_from_cpp<bool>(left_value>right_value);
                } break;
                case nds::StringExprType: {
                    std::string left_value  = get_cpp_from_ast<std::string>(left);
                    std::string right_value = get_cpp_from_ast<std::string>(right);
                    return get_ast_from_cpp<bool>(left_value.size()>right_value.size());
                } break;
                case nds::BoolExprType: {
                    bool left_value  = get_cpp_from_ast<bool>(left);
                    bool right_value = get_cpp_from_ast<bool>(right);
                    return get_ast_from_cpp<bool>(left_value>right_value);
                } break;
                default:
                    throw excptn::PurahError("Unsupported type for > operator");
                    break;
            }
        } break;
        case tkn::LESS: {
            if(left->nodeType() != right->nodeType())
                throw excptn::PurahError("Nodes should has the same type to be handeled by operator");
            switch(left->nodeType()) {
                case nds::IntExprType: {
                    int64_t left_value  = get_cpp_from_ast<int64_t>(left);
                    int64_t right_value = get_cpp_from_ast<int64_t>(right);
                    return get_ast_from_cpp<bool>(left_value<right_value);
                } break;
                case nds::FloatExprType: {
                    double left_value  = get_cpp_from_ast<double>(left);
                    double right_value = get_cpp_from_ast<double>(right);
                    return get_ast_from_cpp<bool>(left_value<right_value);
                } break;
                case nds::StringExprType: {
                    std::string left_value  = get_cpp_from_ast<std::string>(left);
                    std::string right_value = get_cpp_from_ast<std::string>(right);
                    return get_ast_from_cpp<bool>(left_value.size()<right_value.size());
                } break;
                case nds::BoolExprType: {
                    bool left_value  = get_cpp_from_ast<bool>(left);
                    bool right_value = get_cpp_from_ast<bool>(right);
                    return get_ast_from_cpp<bool>(left_value<right_value);
                } break;
                default:
                    throw excptn::PurahError("Unsupported type for < operator");
                    break;
            }
        } break;
        case tkn::MORE_OR_EQUALITY: {
            if(left->nodeType() != right->nodeType())
                throw excptn::PurahError("Nodes should has the same type to be handeled by operator");
            switch(left->nodeType()) {
                case nds::IntExprType: {
                    int64_t left_value  = get_cpp_from_ast<int64_t>(left);
                    int64_t right_value = get_cpp_from_ast<int64_t>(right);
                    return get_ast_from_cpp<bool>(left_value>=right_value);
                } break;
                case nds::FloatExprType: {
                    double left_value  = get_cpp_from_ast<double>(left);
                    double right_value = get_cpp_from_ast<double>(right);
                    return get_ast_from_cpp<bool>(left_value>=right_value);
                } break;
                case nds::StringExprType: {
                    std::string left_value  = get_cpp_from_ast<std::string>(left);
                    std::string right_value = get_cpp_from_ast<std::string>(right);
                    return get_ast_from_cpp<bool>(left_value.size()>=right_value.size());
                } break;
                case nds::BoolExprType: {
                    bool left_value  = get_cpp_from_ast<bool>(left);
                    bool right_value = get_cpp_from_ast<bool>(right);
                    return get_ast_from_cpp<bool>(left_value>=right_value);
                } break;
                default:
                    throw excptn::PurahError("Unsupported type for >= operator");
                    break;
            }
        } break;
        case tkn::LESS_OR_EQUALITY: {
            if(left->nodeType() != right->nodeType())
                throw excptn::PurahError("Nodes should has the same type to be handeled by operator");
            switch(left->nodeType()) {
                case nds::IntExprType: {
                    int64_t left_value  = get_cpp_from_ast<int64_t>(left);
                    int64_t right_value = get_cpp_from_ast<int64_t>(right);
                    return get_ast_from_cpp<bool>(left_value<=right_value);
                } break;
                case nds::FloatExprType: {
                    double left_value  = get_cpp_from_ast<double>(left);
                    double right_value = get_cpp_from_ast<double>(right);
                    return get_ast_from_cpp<bool>(left_value<=right_value);
                } break;
                case nds::StringExprType: {
                    std::string left_value  = get_cpp_from_ast<std::string>(left);
                    std::string right_value = get_cpp_from_ast<std::string>(right);
                    return get_ast_from_cpp<bool>(left_value.size()<=right_value.size());
                } break;
                case nds::BoolExprType: {
                    bool left_value  = get_cpp_from_ast<bool>(left);
                    bool right_value = get_cpp_from_ast<bool>(right);
                    return get_ast_from_cpp<bool>(left_value<=right_value);
                } break;
                default:
                    throw excptn::PurahError("Unsupported type for <= operator");
                    break;
            }
        } break;
        case tkn::NO_EQUALITY: {
            if(left->nodeType() != right->nodeType())
                throw excptn::PurahError("Nodes should has the same type to be handeled by operator");
            switch(left->nodeType()) {
                case nds::IntExprType: {
                    int64_t left_value  = get_cpp_from_ast<int64_t>(left);
                    int64_t right_value = get_cpp_from_ast<int64_t>(right);
                    return get_ast_from_cpp<bool>(left_value!=right_value);
                } break;
                case nds::FloatExprType: {
                    double left_value  = get_cpp_from_ast<double>(left);
                    double right_value = get_cpp_from_ast<double>(right);
                    return get_ast_from_cpp<bool>(left_value!=right_value);
                } break;
                case nds::StringExprType: {
                    std::string left_value  = get_cpp_from_ast<std::string>(left);
                    std::string right_value = get_cpp_from_ast<std::string>(right);
                    return get_ast_from_cpp<bool>(left_value!=right_value);
                } break;
                case nds::BoolExprType: {
                    bool left_value  = get_cpp_from_ast<bool>(left);
                    bool right_value = get_cpp_from_ast<bool>(right);
                    return get_ast_from_cpp<bool>(left_value!=right_value);
                } break;
                default:
                    throw excptn::PurahError("Unsupported type for != operator");
                    break;
            }
        } break;
        default:
            throw excptn::PurahError("I can't handle this operator yet");
            break;
    }
    /// Boo!~ ///
    return std::make_shared<nds::ASTNode>();
}