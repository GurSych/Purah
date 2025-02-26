#include <iostream>
#include <string>

#include "Purah.hpp"

int main(int argc, char* argv[]) {
    std::cout << "Welcome back to Purah!'" << std::endl;
    if(argc < 2) {
        std::cout << "|| Purah: can't find file name" << std::endl;
        return 1;
    }
    if(argc > 2) {
        std::cout << "|| Too many arguments" << std::endl;
        return 1;
    }
    std::ifstream file{argv[1]};
    if(!file) {
        std::cout << "|| Purah: can't open '"
            << argv[1] << "' file" << std::endl;
        return 1;
    }
    Purah purah{};
    std::string input{};
    while(std::getline(file,input)) {
        std::cout << input << std::endl;
    }
    return 0;
}