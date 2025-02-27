#pragma once
#include <exception>
#include <string>

class LexerError: public std::exception {
    public:
        LexerError(const char* _msg): message{_msg} {}
        LexerError(const std::string& _msg): message{_msg} {}
        const char* what() const throw() {
            return (std::string{"|| Lexer error! " + message}).c_str();
        }
    private:
        std::string message{};
};
class MemoryError: public std::exception {
    public:
        MemoryError(const char* _msg): message{_msg} {}
        MemoryError(const std::string& _msg): message{_msg} {}
        const char* what() const throw() {
            return (std::string{"|| Memory error! " + message}).c_str();
        }
    private:
        std::string message{};
};