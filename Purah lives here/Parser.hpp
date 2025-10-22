#pragma once
#include <stdint.h>
#include <vector>
#include <memory>
#include <map>

#ifdef EOF
    #undef EOF
#endif

#include "ParserNodes.hpp"
#include "Exceptions.hpp"
#include "Tokens.hpp"

using namespace purah;

namespace purah { namespace prsr {

    class Parser {
    public:
        Parser(std::vector<tkn::Token>& _tokens) : tokens{_tokens} {
            token_iter = tokens.cbegin();
            token_end = tokens.cend();
        }
        void operator()() {
            while (available()) {
                if(token_iter->type == tkn::EOF || token_iter->type == tkn::EOL) {
                    go_next_token();
                    continue;
                }
                if(token_iter->type == tkn::FUNCTION) {
                    func_vector.push_back(std::move(parse_func_statement()));
                    continue;
                }
                nds::ASTPtr node = parse_statement();
                if(node->nodeType() == nds::IdentifierExprType && available())
                    go_next_token();
                if(node->nodeType() != nds::SimpleAST)
                    AST_vector.push_back(std::move(node));
            }
        }
        bool available() {
            return (token_iter < token_end);
        }
        nds::ASTPtr parse_statement() {
            if(!available())
                return std::move(nds::ASTPtr{new nds::ASTNode{}});
            nds::ASTPtr node{};
            switch(token_iter->type) {
                case tkn::FUNCTION:
                    node = parse_func_statement();
                    break;
                case tkn::RETURN:
                    node = parse_return_statement();
                    break;
                case tkn::IDENTIFIER:
                    node = parse_identifier_statement();
                    break;
                case tkn::VAR:
                    node = parse_var_statement();
                    break;
                case tkn::IF:
                    node = parse_if_statement();
                    break;
                case tkn::COUT:
                    node = parse_COUT_statement();
                    break;
                default:
                    node = parse_expression_statement();
                    break;
            }
            return std::move(node);
        }
        static inline const std::map<tkn::TokenType,unsigned int> token_priority{
            {tkn::EMPTY,0},
            {tkn::EQUALITY,10},{tkn::NO_EQUALITY,10},
            {tkn::MORE,15},{tkn::LESS,15},
            {tkn::MORE_OR_EQUALITY,15},{tkn::LESS_OR_EQUALITY,15},
            {tkn::PLUS,20},{tkn::MINUS,20},
            {tkn::STAR,30},{tkn::SLASH,30},{tkn::DBL_SLASH,30},{tkn::PERCENT,30}
        };
        std::vector<nds::ASTPtr> AST_vector{};
        std::vector<nds::ASTPtr> func_vector{};
    private:
        std::vector<tkn::Token>& tokens;
        std::vector<tkn::Token>::const_iterator token_iter;
        std::vector<tkn::Token>::const_iterator token_end;
        void check_nds_type(tkn::TokenType type1, tkn::TokenType type2, std::string error) {
            if(type1 != type2)
                throw excptn::ParserError(error);
        }
        void go_next_token() {
            if(!available())
                throw excptn::ParserError("Unexpected end of tokens at line: " + std::to_string((token_iter-1)->line));
            ++token_iter;
        }
        void go_check_type(tkn::TokenType type, std::string error) {
            go_next_token();
            check_nds_type(token_iter->type, type, error);
        }
        nds::ASTPtr parse_identifier_statement() {
            std::string id_name   = token_iter->value;
            nds::ASTPtr name_node = std::make_shared<nds::IdentifierExprNode>(id_name);
            if(available()) {
                go_next_token();
                if(token_iter->type == tkn::EQUALS) {
                    go_next_token();
                    nds::ASTPtr value_node = parse_expression_statement();
                    nds::ASTPtr node       = std::make_shared<nds::BinaryExprNode>(tkn::EQUALS, std::move(name_node), std::move(value_node));
                    return std::move(node);
                }
                else if(token_iter->type == tkn::L_BRACKET) {
                    go_next_token();
                    std::vector<nds::ASTPtr> call_args{};
                    while(token_iter->type != tkn::R_BRACKET) {
                        nds::ASTPtr arg = parse_expression_statement();
                        call_args.push_back(std::move(arg));
                        if(token_iter->type == tkn::COMMA) go_next_token();
                        else if(token_iter->type != tkn::R_BRACKET)
                            throw excptn::ParserError("Loosing call argument at line: " + std::to_string(token_iter->line));
                    }
                    go_next_token();
                    return std::make_shared<nds::CallExprNode>(id_name, std::move(call_args));
                }
            }
            return std::move(name_node);
        }
        nds::ASTPtr parse_expression_statement(unsigned int min_priority = 0u) {
            nds::ASTPtr left = parse_primary_expression();
            while(available() && tkn::is_binary_operator(token_iter->type) && (token_priority.at(token_iter->type) >= min_priority)) {
                tkn::TokenType op = token_iter->type;
                unsigned int op_priority = token_priority.at(op);
                go_next_token();
                nds::ASTPtr right = parse_expression_statement(op_priority);
                nds::ASTPtr node = std::make_shared<nds::BinaryExprNode>(op, std::move(left), std::move(right));
                return std::move(node);
            }
            return std::move(left);
        }
        nds::ASTPtr parse_primary_expression() {
            if(token_iter->type == tkn::INTEGER) {
                int64_t value = std::stoll(token_iter->value);
                go_next_token();
                return std::make_shared<nds::IntExprNode>(value);
            }
            if(token_iter->type == tkn::FLOAT) {
                double value = std::stod(token_iter->value);
                go_next_token();
                return std::make_shared<nds::FloatExprNode>(value);
            }
            if(token_iter->type == tkn::BOOL) {
                bool value = (token_iter->value == "true" ? true : false);
                go_next_token();
                return std::make_shared<nds::BoolExprNode>(value);
            }
            else if(token_iter->type == tkn::STRING) {
                std::string value = token_iter->value;
                go_next_token();
                return std::make_shared<nds::StringExprNode>(value);
            }
            else if(token_iter->type == tkn::IDENTIFIER) {
                return parse_identifier_statement();
            }
            else if(token_iter->type == tkn::L_BRACKET) {
                go_next_token();
                nds::ASTPtr expr = parse_expression_statement();
                if(token_iter->type != tkn::R_BRACKET)
                    throw excptn::ParserError("Loosing closing bracket at line: " + std::to_string(token_iter->line));
                go_next_token();
                return std::move(expr);
            }
            else {
                throw excptn::ParserError("Loosing primary expression at line: " + std::to_string(token_iter->line));
            }
        }
        nds::ASTPtr parse_var_statement() {
            go_check_type(tkn::IDENTIFIER,  "Loosing new variable name at line: " + std::to_string(token_iter->line));
            std::string var_name = token_iter->value;
            go_check_type(tkn::BACKSLASH,  "Loosing new variable type at line: " + std::to_string(token_iter->line));
            go_check_type(tkn::IDENTIFIER, "Loosing new variable type at line: " + std::to_string(token_iter->line));
            std::string var_type = token_iter->value;
            go_check_type(tkn::EQUALS, "Loosing new variable value at line: " + std::to_string(token_iter->line));
            go_next_token();
            nds::ASTPtr var_value = parse_expression_statement();
            return std::make_shared<nds::NewVarNode>(var_type, var_name, std::move(var_value));
        }
        nds::ASTPtr parse_func_statement() {
            go_check_type(tkn::IDENTIFIER,  "Loosing new function name at line: " + std::to_string(token_iter->line));
            std::string func_name = token_iter->value;
            go_check_type(tkn::BACKSLASH,  "Loosing new function returning type at line: " + std::to_string(token_iter->line));
            go_check_type(tkn::IDENTIFIER, "Loosing new function returning type at line: " + std::to_string(token_iter->line));
            std::string returning_type = token_iter->value;
            go_check_type(tkn::L_BRACKET, "Loosing new function argumets at line: " + std::to_string(token_iter->line));
            std::vector<nds::ASTPtr> func_args{};
            go_next_token();
            if(token_iter->type == tkn::IDENTIFIER) {
                func_args.push_back(std::move(_parse_arg()));
                while(token_iter->type == tkn::COMMA) {
                    go_check_type(tkn::IDENTIFIER, "Loosing argument at line: " + std::to_string(token_iter->line));
                    func_args.push_back(std::move(_parse_arg()));
                }
            }
            if(token_iter->type != tkn::R_BRACKET)
                throw excptn::ParserError("Loosing closing bracket at line: " + std::to_string(token_iter->line));
            go_check_type(tkn::L_CRL_BRACKET, "Loosing new function body at line: " + std::to_string(token_iter->line));
            std::vector<nds::ASTPtr> func_body{};
            go_next_token();
            while (token_iter->type != tkn::R_CRL_BRACKET) {
                func_body.push_back(std::move(parse_statement()));
            }
            go_next_token();
            return std::make_shared<nds::FunctionExprNode>(func_name,returning_type,std::move(func_args),std::move(func_body));
        }
        auto _parse_arg() -> nds::ASTPtr {
            std::string arg_name = token_iter->value;
            go_check_type(tkn::BACKSLASH,  "Loosing argument type at line: " + std::to_string(token_iter->line));
            go_check_type(tkn::IDENTIFIER, "Loosing argument type at line: " + std::to_string(token_iter->line));
            std::string arg_type = token_iter->value;
            go_next_token();
            return std::make_shared<nds::TypedIdentifierExprNode>(arg_name,arg_type);
        }
        nds::ASTPtr parse_return_statement() {
            go_next_token();
            nds::ASTPtr value = parse_expression_statement();
            return std::make_shared<nds::ReturnExprNode>(std::move(value));
        }
        nds::ASTPtr parse_if_statement() {
            go_check_type(tkn::L_BRACKET, "Loosing bool if-condition at line: " + std::to_string(token_iter->line));
            go_next_token();
            nds::ASTPtr condition = parse_expression_statement();
            if(token_iter->type != tkn::R_BRACKET)
                throw excptn::ParserError("Loosing closing bracket at line: " + std::to_string(token_iter->line));
            go_check_type(tkn::L_CRL_BRACKET, "Loosing if-statement body at line: " + std::to_string(token_iter->line));
            std::vector<nds::ASTPtr> body;
            go_next_token();
            while (token_iter->type != tkn::R_CRL_BRACKET) {
                body.push_back(std::move(parse_statement()));
            }
            go_next_token();
            return std::make_shared<nds::IfExprNode>(std::move(condition),std::move(body),std::move(_parse_else()));
        }
        auto _parse_else() -> nds::ASTPtr {
            if(token_iter->type == tkn::ELSE) {
                bool else_if{false};
                go_next_token();
                nds::ASTPtr condition = std::make_shared<nds::BoolExprNode>(true);
                if(token_iter->type == tkn::IF) {
                    go_check_type(tkn::L_BRACKET, "Loosing bool if-condition at line: " + std::to_string(token_iter->line));
                    go_next_token();
                    condition.reset();
                    condition = std::move(parse_expression_statement());
                    else_if = true;
                    if(token_iter->type != tkn::R_BRACKET)
                        throw excptn::ParserError("Loosing closing bracket at line: " + std::to_string(token_iter->line));
                    go_next_token();
                }
                if(token_iter->type != tkn::L_CRL_BRACKET)
                    throw excptn::ParserError("Loosing if-statement body at line: " + std::to_string(token_iter->line));
                std::vector<nds::ASTPtr> body;
                go_next_token();
                while (token_iter->type != tkn::R_CRL_BRACKET) {
                    body.push_back(std::move(parse_statement()));
                }
                go_next_token();
                nds::ASTPtr next_node = else_if ? std::move(_parse_else()) : std::make_shared<nds::ASTNode>();
                return std::make_shared<nds::IfExprNode>(std::move(condition),std::move(body),std::move(next_node));
            }
            else return std::make_shared<nds::ASTNode>();
        }
        nds::ASTPtr parse_COUT_statement() {
            go_next_token();
            nds::ASTPtr value = parse_expression_statement();
            return std::make_shared<nds::COUTExprNode>(std::move(value));
        }
    };

} }