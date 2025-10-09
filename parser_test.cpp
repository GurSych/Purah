#include <iostream>
#include <string>
#include <map>
#include "Purah.hpp"

using namespace purah;

void print(std::vector<tkn::Token> vec) {
    std::cout << vec[0].line << ": ";
    unsigned int i{};
    for(tkn::Token& token : vec)
        std::cout << (i++ ? ", <" : "<") << token.type << " '" << token.value << "'>";
    std::cout << std::endl;
}

const std::map<nds::ASTNodeType, std::string> keywords{
    {nds::SimpleAST,"SimpleAST"},
    {nds::NewVarNodeType,"NewVarNodeType"},
    {nds::IdentifierExprType,"IdentifierExprType"},
    {nds::IntExprType,"IntExprType"},
    {nds::FloatExprType,"FloatExprType"},
    {nds::BoolExprType,"BoolExprType"},
    {nds::StringExprType,"StringExprType"},
    {nds::BinaryExprType,"BinaryExprType"},
    {nds::CallExprType,"CallExprType"},
    {nds::COUTExprType,"COUTExprType"}
};

int main() {
    std::string input{};
    std::string str{};
    unsigned int line{};
    while(true) {
        std::getline(std::cin,str);
        if(str == "quit") break;
        input += str + '\n';
    }
    std::vector<tkn::Token> tokens = lxr::Lexer{}(input);
    print(tokens);
    prsr::Parser parser{tokens};
    parser();
    std::vector<nds::ASTPtr>&& ast = std::move(parser.func_vector);
    std::cout << "Parsed " << ast.size() << " functions." << std::endl;
    //for(unsigned int i{}; i < ast.size(); ++i)
    //    std::cout << i <<". Node type: " << keywords.at(ast.at(i)->nodeType()) << std::endl;
    return 0;
}
