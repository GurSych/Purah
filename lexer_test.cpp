#include <iostream>
#include "Purah.hpp"

using namespace purah;

void print(std::vector<tkn::Token> vec) {
    std::cout << vec[0].line << ": ";
    unsigned int i{};
    for(tkn::Token& token : vec)
        std::cout << (i++ ? ", <" : "<") << token.type << " '" << token.value << "'>";
    std::cout << std::endl;
}

int main() {
    lxr::Lexer lexer{};
    std::string str{};
    unsigned int line{};
    while(str != "quit") {
        std::cout << ">>> "; std::getline(std::cin,str);
        print(lexer(str,++line));
    }
    return 0;
}