#include <iostream>
#include "Purah.hpp"

void print(std::vector<Token> vec) {
    std::cout << vec[0].line << ": ";
    unsigned int i{};
    for(Token& token : vec)
        std::cout << (i++ ? ", <" : "<") << token.type << " '" << token.value << "'>";
    std::cout << std::endl;
}

int main() {
    Lexer lexer{};
    std::string str{};
    unsigned int line{};
    while(str != "quit") {
        std::cout << ">>> "; std::getline(std::cin,str);
        print(lexer(str,++line));
    }
    return 0;
}