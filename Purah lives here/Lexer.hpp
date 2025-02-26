#pragma once
#include <exception>
#include <vector>
#include <string>
#include <cctype>
#include <regex>
#include <map>

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
    PRINT
};

struct Token {
    Token(TokenType _t, std::string _v, size_t _l) : 
        type{_t}, value{_v}, line{_l} {}
    TokenType type{};
    std::string value{};
    size_t line{};
};

class Lexer {
public:
    Lexer() {}
    std::vector<Token> operator()(const std::string& input, size_t line = 0ull) {
        std::vector<Token> tokens{};
        size_t pos{}; size_t input_size = input.size();
        while(pos < input_size) {
            char c = input[pos];
            if(isspace(c)) {}
            else if(std::isdigit(c)) {
                std::string num{};
                bool isfloat{};
                while(isdigit(input[pos]) && pos < input_size) num += input[pos++];
                if(pos < input_size && input[pos] == '.') {
                    isfloat = true; num += "."; ++pos;
                    while(isdigit(input[pos]) && pos < input_size) num += input[pos++];
                    if(pos < input_size) if(input[pos] == '.')
                        throw std::runtime_error("Unexpected dot at line " + std::to_string(line));
                }
                tokens.emplace_back((isfloat ? FLOAT : NUMBER),num,line); --pos;
            }
            else if(std::isalpha(c)) {
                std::string name{};
                while(isalpha(input[pos]) && pos < input_size) name += input[pos++];
                --pos;
                if(name == "true" || name == "false") tokens.emplace_back(BOOl,name,line);
                else if(system_words.count(name)) tokens.emplace_back(system_words.at(name),name,line);
                else tokens.emplace_back(IDENTIFIER,name,line);
            }
            else if(c == '.') tokens.emplace_back(DOT,".",line);
            else if(c == ',') tokens.emplace_back(COMMA,",",line);
            else if(c == ';') tokens.emplace_back(SEMICOLON,";",line);
            else if(c == '"') {
                std::string str{}; ++pos;
                while(input[pos] != '"' && pos < input_size) 
                    str += input[pos++];
                if(pos >= input_size) 
                    throw std::runtime_error("Unclosed string at line " + std::to_string(line));
                tokens.emplace_back(STRING,str,line);
            }
            else if(c == '(') tokens.emplace_back(LPAREN,"(",line);
            else if(c == ')') tokens.emplace_back(RPAREN,")",line);
            else if(c == '=') {
                if(pos < input_size && input[pos+1] == '=') {
                    tokens.emplace_back(EQUALITY,"==",line); ++pos;
                }
                else tokens.emplace_back(EQUALS,"=",line);
            }
            else if(c == '!') {
                if(pos < input_size && input[pos+1] == '=') {
                    tokens.emplace_back(NO_EQALITY,"!=",line); ++pos;
                }
                else tokens.emplace_back(NOT,"!",line);
            }
            else if(c == '+') {
                if(pos < input_size && input[pos+1] == '+') {
                    tokens.emplace_back(INCRIMENT,"++",line); ++pos;
                }
                else tokens.emplace_back(PLUS,"+",line);
            }
            else if(c == '-') {
                if(pos < input_size && input[pos+1] == '-') {
                    tokens.emplace_back(DECRIMENT,"--",line); ++pos;
                }
                if(pos < input_size && input[pos+1] == '>') {
                    tokens.emplace_back(ARROW,"->",line); ++pos;
                }
                else tokens.emplace_back(MINUS,"-",line);
            }
            else if(c == '*') tokens.emplace_back(STAR,"*",line);
            else if(c == '/') {
                if(pos < input_size && input[pos+1] == '/') {
                    tokens.emplace_back(FLOOR_DIVIDE,"//",line); ++pos;
                }
                else tokens.emplace_back(DIVIDE,"/",line);
            }
            else if(c == '%') tokens.emplace_back(PERCENT,"%",line);
            else if(c == '<') {
                if(pos < input_size && input[pos+1] == '=') {
                    tokens.emplace_back(LESS_OR_EQALITY,"<=",line); ++pos;
                }
                else tokens.emplace_back(LESS,"<",line);
            }
            else if(c == '>') {
                if(pos < input_size && input[pos+1] == '=') {
                    tokens.emplace_back(MORE_OR_EQALITY,">=",line); ++pos;
                }
                else tokens.emplace_back(MORE,">",line);
            }
            else if(c == '{') tokens.emplace_back(LFIGPAREN,"{",line);
            else if(c == '}') tokens.emplace_back(RFIGPAREN,"}",line);
            else throw std::runtime_error("Unexpected input at line " + std::to_string(line));
            ++pos;
        }
        tokens.emplace_back(EOF,"",line);
        return tokens;
    }
private:
    const std::map<std::string,TokenType> system_words{
        {"func",FUNCTION},{"return",RETURN},
        {"if",IF},{"else",ELSE},//{"true",TRUE},{"false",FALSE},
        {"not",NOT},{"and",AND},{"or",OR},
        {"while",WHILE},{"for",FOR},
        {"print",PRINT}
    };
};