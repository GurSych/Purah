#pragma once
#include <vector>
#include <string>
#include <cctype>
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
    friend class Purah;
        Lexer() {}
        std::vector<tkn::Token> operator()(const std::string& input, size_t s_line = 0ull) {
            std::vector<tkn::Token> tokens{};
            //size_t& line = (s_line == 0ull ? s_line : current_line);
            size_t line = s_line;
            std::string::const_iterator iter = input.cbegin();
            std::string::const_iterator end  = input.cend();
            while(iter < end) {
                char c = *iter;
                if(isspace(c)) {
                    if(c == '\n') tokens.emplace_back(tkn::EOL,"\\n",line++);
                }
                else if(std::isdigit(c)) {
                    std::string num{};
                    bool isfloat{};
                    while(isdigit(*iter) && iter < end) num += *iter++;
                    if(iter < end && *iter == '.') {
                        isfloat = true; num += "."; ++iter;
                        while(isdigit(*iter) && iter < end) num += *iter++;
                        if(iter < end && *iter == '.')
                            throw excptn::LexerError(std::string{"Unexpected dot at line "} + std::to_string(line));
                    }
                    tokens.emplace_back((isfloat ? tkn::FLOAT : tkn::INTEGER),num,line); --iter;
                }
                else if(std::isalpha(c)) {
                    std::string name{};
                    while((std::isalpha(*iter) || std::isdigit(*iter) || *iter == '_') && iter < end)
                        name += *iter++;
                    if(iter < end) --iter;
                    if(name == "true" || name == "false") tokens.emplace_back(tkn::BOOL,name,line);
                    else if(system_words.count(name)) tokens.emplace_back(system_words.at(name),name,line);
                    else tokens.emplace_back(tkn::IDENTIFIER,name,line);
                }
                else if(c == '.') tokens.emplace_back(tkn::DOT,".",line);
                else if(c == ',') tokens.emplace_back(tkn::COMMA,",",line);
                else if(c == ':') tokens.emplace_back(tkn::COLON,":",line);
                else if(c == ';') tokens.emplace_back(tkn::SEMICOLON,";",line);
                else if(c == '"') {
                    std::string str{}; ++iter;
                    while(*iter != '"' && iter < end) {
                        static bool special_char = false;
                        if(special_char) {
                            if(*iter == 'n')       str += '\n';
                            else if(*iter == 't')  str += '\t';
                            else if(*iter == '"')  str += '"';
                            else if(*iter == '\'') str += '\'';
                            else if(*iter == '\\') str += '\\';
                            else if(*iter == 'r')  str += '\r';
                            else if(*iter == '0')  str += '\0';
                            else if(*iter == 'a')  str += '\a';
                            else if(*iter == 'b')  str += '\b';
                            else if(*iter == 'f')  str += '\f';
                            else if(*iter == 'v')  str += '\v';
                            else 
                                throw excptn::LexerError("Unknown special character at line " + std::to_string(line));
                            special_char = false; ++iter;
                            continue;
                        }
                        if(*iter == '\\' && !special_char) {
                            special_char = true; ++iter;
                            continue;
                        }
                        str += *iter++;
                    }
                    if(iter >= end) 
                        throw excptn::LexerError("Unclosed string at line " + std::to_string(line));
                    tokens.emplace_back(tkn::STRING,str,line);
                }
                else if(c == '\\') {
                    if(iter < end && *(iter+1) == '\\') {
                        tokens.emplace_back(tkn::DBL_BACKSLASH,"\\\\",line); ++iter;
                    }
                    else tokens.emplace_back(tkn::BACKSLASH,"\\",line);
                }
                else if(c == '(') tokens.emplace_back(tkn::L_BRACKET,"(",line);
                else if(c == ')') tokens.emplace_back(tkn::R_BRACKET,")",line);
                else if(c == '=') {
                    if(iter < end && *(iter+1) == '=') {
                        tokens.emplace_back(tkn::EQUALITY,"==",line); ++iter;
                    }
                    else tokens.emplace_back(tkn::EQUALS,"=",line);
                }
                else if(c == '!') {
                    if(iter < end && *(iter+1) == '=') {
                        tokens.emplace_back(tkn::NO_EQUALITY,"!=",line); ++iter;
                    }
                    else tokens.emplace_back(tkn::NOT,"!",line);
                }
                else if(c == '+') {
                    if(iter < end && *(iter+1) == '+') {
                        tokens.emplace_back(tkn::INCREMENT,"++",line); ++iter;
                    }
                    else tokens.emplace_back(tkn::PLUS,"+",line);
                }
                else if(c == '-') {
                    if(iter < end && *(iter+1) == '-') {
                        tokens.emplace_back(tkn::DECREMENT,"--",line); ++iter;
                    }
                    if(iter < end && *(iter+1) == '>') {
                        tokens.emplace_back(tkn::ARROW,"->",line); ++iter;
                    }
                    else tokens.emplace_back(tkn::MINUS,"-",line);
                }
                else if(c == '*') tokens.emplace_back(tkn::STAR,"*",line);
                else if(c == '/') {
                    if(iter < end && *(iter+1) == '/') {
                        tokens.emplace_back(tkn::DBL_SLASH,"//",line); ++iter;
                    }
                    else tokens.emplace_back(tkn::SLASH,"/",line);
                }
                else if(c == '%') tokens.emplace_back(tkn::PERCENT,"%",line);
                else if(c == '<') {
                    if(iter < end && *(iter+1) == '=') {
                        tokens.emplace_back(tkn::LESS_OR_EQUALITY,"<=",line); ++iter;
                    }
                    else tokens.emplace_back(tkn::LESS,"<",line);
                }
                else if(c == '>') {
                    if(iter < end && *(iter+1) == '=') {
                        tokens.emplace_back(tkn::MORE_OR_EQUALITY,">=",line); ++iter;
                    }
                    else tokens.emplace_back(tkn::MORE,">",line);
                }
                else if(c == '{') tokens.emplace_back(tkn::L_CRL_BRACKET,"{",line);
                else if(c == '}') tokens.emplace_back(tkn::R_CRL_BRACKET,"}",line);
                else if(c == '[') tokens.emplace_back(tkn::L_SQR_BRACKET,"{",line);
                else if(c == ']') tokens.emplace_back(tkn::R_SQR_BRACKET,"}",line);
                else if(c == '~') {
                    do { ++iter; } while(*iter != '~');
                }
                else throw excptn::LexerError("Unexpected input " + std::string{c} + " at line " + std::to_string(line));
                ++iter;
            }
            //tokens.emplace_back(tkn::EOF,"",line);
            return tokens;
        }
        void tokenize(const std::string& input, size_t line) {
            std::vector<tkn::Token> new_tokens = operator()(input,line);
            tokens.insert(tokens.cend(),new_tokens.cbegin(),new_tokens.cend());
        }
        std::vector<tkn::Token>& get_tokens() {
            return tokens;
        }
        void clear() {
            tokens.clear();
            current_line = 1ull;
        }
        static const inline std::map<std::string,tkn::TokenType> system_words{
            {"var",tkn::VAR},{"const",tkn::CONST},
            {"func",tkn::FUNCTION},{"return",tkn::RETURN},
            {"if",tkn::IF},{"else",tkn::ELSE},
            {"not",tkn::NOT},{"and",tkn::AND},{"or",tkn::OR},
            {"while",tkn::WHILE},{"for",tkn::FOR},
            {"COUT",tkn::COUT}
        };
    private:
        std::vector<tkn::Token> tokens{};
        size_t current_line{1ull};
    };

} }