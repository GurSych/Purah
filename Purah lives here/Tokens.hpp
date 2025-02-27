#pragma once
#include <string>

#ifdef EOF
    #undef EOF
#endif

enum TokenType {
    EOF = -1, IDENTIFIER,
    NUMBER, STRING, FLOAT, BOOl,
    DOT, COMMA, SEMICOLON,
    PLUS, MINUS, EQUALS, LPAREN, RPAREN,
    STAR, DIVIDE, FLOOR_DIVIDE, PERCENT,
    IF, ELSE, FUNCTION, RETURN,
    LFIGPAREN, RFIGPAREN, ARROW,
    INCRIMENT, DECRIMENT,
    NOT, AND, OR, TRUE, FALSE,
    EQUALITY, NO_EQALITY, MORE, MORE_OR_EQALITY, LESS, LESS_OR_EQALITY,
    WHILE, FOR,
    PRINT, CLASS
};

struct Token {
    Token(TokenType _t, std::string _v, size_t _l) : 
        type{_t}, value{_v}, line{_l} {}
    TokenType type{};
    std::string value{};
    size_t line{};
};