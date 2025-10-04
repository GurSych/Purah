#pragma once
#include <stdint.h>
#include <vector>
#include <memory>
#include <iostream>

#ifdef EOF
    #undef EOF
#endif

#include "ParserNodes.hpp"
#include "Exceptions.hpp"
#include "Tokens.hpp"
#include "Lexer.hpp"

using namespace purah;

namespace purah { namespace prsr {

    class Parser {
    public:
        Parser(std::vector<tkn::Token>& _tokens) : tokens{_tokens} {
            token_iter = tokens.cbegin();
            token_end = tokens.cend();
        }
        std::vector<nds::ASTPtr> operator()() {
            std::vector<nds::ASTPtr> AST_vector{};
            while (available()) {
                if(token_iter->type == tkn::EOF) {
                    go_next_token();
                    continue;
                }
                nds::ASTPtr node = parse_statement();
                if(node->nodeType() == nds::IdentifierExprType && available())
                    go_next_token();
                if(node->nodeType() != nds::SimpleAST)
                    AST_vector.push_back(std::move(node));
            }
            return std::move(AST_vector);
        }
        bool available() {
            return (token_iter < token_end);
        }
        nds::ASTPtr parse_statement() {
            if(!available())
                return std::move(nds::ASTPtr{new nds::ASTNode{}});
            nds::ASTPtr node{};
            switch(token_iter->type) {
                case tkn::IDENTIFIER:
                    node = parse_identifier_statement();
                    break;
                case tkn::VAR:
                    node = parse_var_statement();
                    break;
                default:
                    node = parse_expression_statement();
                    break;
            }
            return std::move(node);
        }
        static inline const std::map<tkn::TokenType,unsigned int> token_priority{
            {tkn::EMPTY,0},
            {tkn::PLUS,20},{tkn::MINUS,20},
            {tkn::STAR,30},{tkn::SLASH,30},{tkn::DBL_SLASH,30},{tkn::PERCENT,30}
        };
    private:
        std::vector<tkn::Token> tokens{};
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
            nds::ASTPtr name_node = std::make_unique<nds::IdentifierExprNode>(id_name);
            if(available()) {
                go_next_token();
                if(token_iter->type == tkn::EQUALS) {
                    go_next_token();
                    nds::ASTPtr value_node = parse_expression_statement();
                    nds::ASTPtr node       = std::make_unique<nds::BinaryExprNode>(tkn::EQUALS, std::move(name_node), std::move(value_node));
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
                    return std::make_unique<nds::CallExprNode>(id_name, std::move(call_args));
                }
                --token_iter;
            }
            return std::move(name_node);
        }
        nds::ASTPtr parse_expression_statement(unsigned int min_priority = 0u) {
            nds::ASTPtr left = parse_primary_expression();
            go_next_token();
            while(available() && tkn::is_binary_operator(token_iter->type) && (token_priority.at(token_iter->type) > min_priority)) {
                tkn::TokenType op = token_iter->type;
                unsigned int op_priority = token_priority.at(op);
                go_next_token();
                nds::ASTPtr right = parse_expression_statement(op_priority);
                nds::ASTPtr node = std::make_unique<nds::BinaryExprNode>(op, std::move(left), std::move(right));
                left = std::move(node);
            }
            return std::move(left);
        }
        nds::ASTPtr parse_primary_expression() {
            if(token_iter->type == tkn::INTEGER) {
                int64_t value = std::stoll(token_iter->value);
                return std::make_unique<nds::IntExprNode>(value);
            }
            if(token_iter->type == tkn::FLOAT) {
                double value = std::stod(token_iter->value);
                return std::make_unique<nds::FloatExprNode>(value);
            }
            if(token_iter->type == tkn::BOOL) {
                bool value = (token_iter->value == "true" ? true : false);
                return std::make_unique<nds::BoolExprNode>(value);
            }
            else if(token_iter->type == tkn::STRING) {
                return std::make_unique<nds::StringExprNode>(token_iter->value);
            }
            else if(token_iter->type == tkn::IDENTIFIER) {
                return parse_identifier_statement();
            }
            else if(token_iter->type == tkn::L_BRACKET) {
                go_next_token();
                nds::ASTPtr expr = parse_expression_statement();
                if(token_iter->type != tkn::R_BRACKET)
                    throw excptn::ParserError("Loosing closing bracket at line: " + std::to_string(token_iter->line));
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
            return std::make_unique<nds::NewVarNode>(var_type, var_name, std::move(var_value));
        }
    };

} }