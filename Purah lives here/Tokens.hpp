#pragma once
#include <string>
#include <set>

#ifdef EOF
    #undef EOF
#endif

namespace purah { namespace tkn {

    enum TokenType {
        EOF = -1, EMPTY,
        IDENTIFIER,
        INTEGER, STRING, FLOAT, BOOL,
        SLASH, DBL_SLASH,                          /*   / //                */  
        BACKSLASH, DBL_BACKSLASH,                  /*   \ \\                */
        PLUS, MINUS, EQUALS, L_BRACKET, R_BRACKET, /*   + - = ( )           */
        INCREMENT, DECREMENT,                      /*   ++ --               */
        STAR, PERCENT,                             /*   * %                 */
        IF, ELSE, FUNCTION, RETURN,                /*   if else func return */
        L_CRL_BRACKET, R_CRL_BRACKET, ARROW,       /*   { } ->              */
        L_SQR_BRACKET, R_SQR_BRACKET,              /*   [ ]                 */
        NOT, AND, OR, TRUE, FALSE,                 /*   ! && || true false  */      
        EQUALITY, NO_EQUALITY,                     /*   == !=               */
        MORE, MORE_OR_EQUALITY,                    /*   > >=                */
        LESS, LESS_OR_EQUALITY,                    /*   < <=                */
        DOT, COMMA, COLON, SEMICOLON,              /*   . , : ;             */
        VAR, CONST,                                /*   var const           */
        WHILE, FOR,                                /*   while for           */  
        COUT, CLASS
    };

    struct Token {
        Token(TokenType _t, std::string _v, size_t _l = 0ull) : 
            type{_t}, value{_v}, line{_l} {}
        TokenType type{};
        std::string value{};
        size_t line{};
    };

    std::set<TokenType> binary_operators { 
        PLUS, MINUS, EQUALS, 
        STAR, SLASH, DBL_SLASH, PERCENT,
        AND, OR, EQUALITY, NO_EQUALITY,
        MORE, MORE_OR_EQUALITY, LESS, LESS_OR_EQUALITY
    };

    bool is_binary_operator(TokenType token_type) {
        return (binary_operators.find(token_type) != binary_operators.end());
    }

    std::set<TokenType> single_value_types { 
        INTEGER, STRING, FLOAT, BOOL
    };

    bool is_single_value_type(TokenType token_type) {
        return (single_value_types.find(token_type) != single_value_types.end());
    }

} }