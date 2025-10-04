#pragma once
#include <exception>
#include <vector>
#include <string>
#include <cctype>
#include <regex>
#include <map>

#include "Exceptions.hpp"

#ifdef EOF
    #undef EOF
#endif

#include "Tokens.hpp"

using namespace purah;

namespace purah { namespace lxr {

    class Lexer {
    public:
        Lexer() {}
        std::vector<tkn::Token> operator()(const std::string& input, size_t line = 0ull) {
            std::vector<tkn::Token> tokens{};
            size_t pos{}; size_t input_size = input.size();
            while(pos < input_size) {
                char c = input[pos];
                if(isspace(c)) {
                    if(c == '\n') tokens.emplace_back(tkn::EOF,"\n",line);
                }
                else if(std::isdigit(c)) {
                    std::string num{};
                    bool isfloat{};
                    while(isdigit(input[pos]) && pos < input_size) num += input[pos++];
                    if(pos < input_size && input[pos] == '.') {
                        isfloat = true; num += "."; ++pos;
                        while(isdigit(input[pos]) && pos < input_size) num += input[pos++];
                        if(pos < input_size) if(input[pos] == '.')
                            throw excptn::LexerError(std::string{"Unexpected dot at line "} + std::to_string(line));
                    }
                    tokens.emplace_back((isfloat ? tkn::FLOAT : tkn::INTEGER),num,line); --pos;
                }
                else if(std::isalpha(c)) {
                    std::string name{};
                    while(isalpha(input[pos]) && pos < input_size) name += input[pos++];
                    --pos;
                    if(name == "true" || name == "false") tokens.emplace_back(tkn::BOOL,name,line);
                    else if(system_words.count(name)) tokens.emplace_back(system_words.at(name),name,line);
                    else tokens.emplace_back(tkn::IDENTIFIER,name,line);
                }
                else if(c == '.') tokens.emplace_back(tkn::DOT,".",line);
                else if(c == ',') tokens.emplace_back(tkn::COMMA,",",line);
                else if(c == ':') tokens.emplace_back(tkn::COLON,":",line);
                else if(c == ';') tokens.emplace_back(tkn::SEMICOLON,";",line);
                else if(c == '"') {
                    std::string str{}; ++pos;
                    while(input[pos] != '"' && pos < input_size) 
                        str += input[pos++];
                    if(pos >= input_size) 
                        throw excptn::LexerError("Unclosed string at line " + std::to_string(line));
                    tokens.emplace_back(tkn::STRING,str,line);
                }
                else if(c == '\\') {
                    if(pos < input_size && input[pos+1] == '\\') {
                        tokens.emplace_back(tkn::DBL_BACKSLASH,"\\\\",line); ++pos;
                    }
                    else tokens.emplace_back(tkn::BACKSLASH,"\\",line);
                }
                else if(c == '(') tokens.emplace_back(tkn::L_BRACKET,"(",line);
                else if(c == ')') tokens.emplace_back(tkn::R_BRACKET,")",line);
                else if(c == '=') {
                    if(pos < input_size && input[pos+1] == '=') {
                        tokens.emplace_back(tkn::EQUALITY,"==",line); ++pos;
                    }
                    else tokens.emplace_back(tkn::EQUALS,"=",line);
                }
                else if(c == '!') {
                    if(pos < input_size && input[pos+1] == '=') {
                        tokens.emplace_back(tkn::NO_EQUALITY,"!=",line); ++pos;
                    }
                    else tokens.emplace_back(tkn::NOT,"!",line);
                }
                else if(c == '+') {
                    if(pos < input_size && input[pos+1] == '+') {
                        tokens.emplace_back(tkn::INCREMENT,"++",line); ++pos;
                    }
                    else tokens.emplace_back(tkn::PLUS,"+",line);
                }
                else if(c == '-') {
                    if(pos < input_size && input[pos+1] == '-') {
                        tokens.emplace_back(tkn::DECREMENT,"--",line); ++pos;
                    }
                    if(pos < input_size && input[pos+1] == '>') {
                        tokens.emplace_back(tkn::ARROW,"->",line); ++pos;
                    }
                    else tokens.emplace_back(tkn::MINUS,"-",line);
                }
                else if(c == '*') tokens.emplace_back(tkn::STAR,"*",line);
                else if(c == '/') {
                    if(pos < input_size && input[pos+1] == '/') {
                        tokens.emplace_back(tkn::DBL_SLASH,"//",line); ++pos;
                    }
                    else tokens.emplace_back(tkn::SLASH,"/",line);
                }
                else if(c == '%') tokens.emplace_back(tkn::PERCENT,"%",line);
                else if(c == '<') {
                    if(pos < input_size && input[pos+1] == '=') {
                        tokens.emplace_back(tkn::LESS_OR_EQUALITY,"<=",line); ++pos;
                    }
                    else tokens.emplace_back(tkn::LESS,"<",line);
                }
                else if(c == '>') {
                    if(pos < input_size && input[pos+1] == '=') {
                        tokens.emplace_back(tkn::MORE_OR_EQUALITY,">=",line); ++pos;
                    }
                    else tokens.emplace_back(tkn::MORE,">",line);
                }
                else if(c == '{') tokens.emplace_back(tkn::L_CRL_BRACKET,"{",line);
                else if(c == '}') tokens.emplace_back(tkn::R_CRL_BRACKET,"}",line);
                else if(c == '[') tokens.emplace_back(tkn::L_SQR_BRACKET,"{",line);
                else if(c == ']') tokens.emplace_back(tkn::R_SQR_BRACKET,"}",line);
                else throw excptn::LexerError("Unexpected input at line " + std::to_string(line));
                ++pos;
            }
            tokens.emplace_back(tkn::EOF,"",line);
            return tokens;
        }
        static const inline std::map<std::string,tkn::TokenType> system_words{
            {"var",tkn::VAR},{"const",tkn::CONST},
            {"func",tkn::FUNCTION},{"return",tkn::RETURN},
            {"if",tkn::IF},{"else",tkn::ELSE},
            {"not",tkn::NOT},{"and",tkn::AND},{"or",tkn::OR},
            {"while",tkn::WHILE},{"for",tkn::FOR},
            {"COUT",tkn::COUT}
        };
    };

} }