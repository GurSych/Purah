#include <iostream>
#include <fstream>
#include <string>

#include "Purah.hpp"

void print_tokens(const std::vector<purah::tkn::Token>& tokens);
void print_stack(const std::stack<purah::Purah::PurahSpace>& stack);

bool DEBUG_MODE  = false;
bool NO_HELLO    = false;
bool YES_GOODBYE = false;
bool COLORED     = false;

int main(int argc, char* argv[]) {
    if(argc > 2) {
        for(unsigned int i = 2u; i < argc; ++i) {
            if(std::string{argv[i]} == "--debug" || std::string{argv[i]} == "-d") {
                DEBUG_MODE = true;
                continue;
            }
            if(std::string{argv[i]} == "--no-hello" || std::string{argv[i]} == "-nh") {
                NO_HELLO = true;
                continue;
            }
            if(std::string{argv[i]} == "--yes-goodbye" || std::string{argv[i]} == "-yg") {
                YES_GOODBYE = true;
                continue;
            }
            if(std::string{argv[i]} == "--colorful") {
                COLORED = true;
                continue;
            }
            std::cout << "|| Purah0: unknown argument '" << argv[i] << "'" << std::endl;
        }
    }
    if(!NO_HELLO) {
        std::cout << "Welcome back to "
            << (COLORED ? "\033[31m":"") 
            << "Purah" << (COLORED ? "\033[0m!":"!") << std::endl;
    }
    if(argc < 2) {
        std::cout << "|| Purah0: can't find file name" << std::endl;
        return -1;
    }
    std::ifstream file{argv[1]};
    if(!file) {
        std::cout << "|| Purah0: can't open '"
            << argv[1] << "' file" << std::endl;
        return -1;
    }
    purah::Purah purah{};
    std::string input{};
    size_t line{};
    while(std::getline(file,input)) {
        try {
            purah.tokenize(input, ++line);
        } catch (const excptn::LexerError& e) {
            if(COLORED) std::cout << "\033[35m" << e.what() << "\033[0m" << std::endl;
            else std::cout << e.what() << std::endl;
            return -1;
        }
    }
    file.close();
    if(DEBUG_MODE) {
        if(COLORED) std::cout << "\033[32m";
        print_tokens(purah.lexer.get_tokens());
        if(COLORED) std::cout << "\033[0m";
    }
    try {
        purah.parse();
    } catch (const excptn::ParserError& e) {
        if(COLORED) std::cout << "\n\033[35m" << e.what() << "\033[0m" << std::endl;
        else std::cout << e.what() << std::endl;
        return -1;
    } catch (const excptn::PurahError& e) {
        if(COLORED) std::cout << "\n\033[35m" << e.what() << "\033[0m" << std::endl;
        else std::cout << e.what() << std::endl;
        return -1;
    }
    if(DEBUG_MODE) {
        if(COLORED) std::cout << "\033[32m";
        std::cout << "Parsing is done!" << std::endl;
        if(COLORED) std::cout << "\033[0m";
    }
    int result{};
    try {
        result = purah.interpret();
    } catch (const excptn::PurahError& e) {
        if(COLORED) std::cout << "\033[35m" << e.what() << "\033[0m" << std::endl;
        else std::cout << e.what() << std::endl;
        return -1;
    } catch (const excptn::MemoryError& e) {
        if(COLORED) std::cout << "\033[35m" << e.what() << "\033[0m" << std::endl;
        else std::cout << e.what() << std::endl;
        return -1;
    }
    if(YES_GOODBYE) {
        std::cout << "\nGoodbye! Have a "
            << (COLORED ? "\033[31m":"") 
            << "great" << (COLORED ? "\033[0m":"") << " day!" << std::endl;
    }
    return result;
}

void print_tokens(const std::vector<purah::tkn::Token>& tokens) {
    std::cout << "|| Debug: Tokens: " << std::endl;
    unsigned int i{};
    for(const purah::tkn::Token& token : tokens)
        std::cout << (i++ ? ", <" : "<") << token.type 
            << " '" << token.value << "' " << token.line << ">";
    std::cout << std::endl;
}

void print_stack(const std::stack<purah::Purah::PurahSpace>& stack) {
    if(COLORED) std::cout << "\033[35m";
    std::cout << "Stack: ";
    // Boo!~ //
    if(COLORED) std::cout << "\033[0m";
}