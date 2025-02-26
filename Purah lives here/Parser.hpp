#pragma once
#include <exception>
#include <vector>
#include <string>
#include <memory>
#include "Lexer.hpp"

struct ASTNode {
    virtual ~ASTNode() = default;
};
struct ExprNode : ASTNode {};