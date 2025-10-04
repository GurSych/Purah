#pragma once
#include <exception>
#include <string>

namespace purah { namespace excptn {

    class LexerError: public std::exception {
        public:
            LexerError(const char* _msg) { message += _msg; }
            LexerError(const std::string& _msg) { message += _msg; }
            const char* what() const throw() { return message.c_str(); }
        private:
            std::string message{"|| Lexer error! "};
    };
    class ParserError: public std::exception {
        public:
            ParserError(const char* _msg) { message += _msg; }
            ParserError(const std::string& _msg) { message += _msg; }
            const char* what() const throw() { return message.c_str(); }
        private:
            std::string message{"|| Parser error! "};
    };
    class MemoryError: public std::exception {
        public:
            MemoryError(const char* _msg) { message += _msg; }
            MemoryError(const std::string& _msg) { message += _msg; }
            const char* what() const throw() { return message.c_str(); }
        private:
            std::string message{"|| Memory error! "};
    };

} }