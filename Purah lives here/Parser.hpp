#pragma once
#include <exception>
#include <vector>
#include <string>
#include <memory>
#include <map>
#include "Lexer.hpp"

struct ASTNode {
    virtual ~ASTNode() = default;
};

class IntExprNode: public ASTNode {
    public:
        IntExprNode(long long int _v) : value{_v} {}
    private:
        long long int value{};
};

class IdentifierExprNode: ASTNode {
    public:
        IdentifierExprNode(const std::string& _n) : name{_n} {}
    private:
        std::string name{};
};

class BinaryExprNode: public ASTNode {
    public:
        BinaryExprNode(TokenType _op, ASTNode* _opl, ASTNode* _opr) 
            : op{_op}, llit{_opl}, rlit(_opr) {}
    private:
        TokenType op{};
        ASTNode* llit{}; ASTNode* rlit{};
};

class CallExprNode: public ASTNode {
    public:
        CallExprNode(const std::string& _n, std::vector<ASTNode*>& _a_s)
            : name{_n}, args{_a_s} {}
    private:
        std::string name{};
        std::vector<ASTNode*> args{};
};

using ASTPtr = std::unique_ptr<ASTNode>;

class Parser {
    public:
        Parser() {}
        ASTPtr operator()(std::vector<Token>& tokens) {
            unsigned long long pos{};
        /// Boo!~ ///
        }
};