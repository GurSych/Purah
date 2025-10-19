#pragma once
#include <string>
#include <map>

#include "ParserNodes.hpp"

namespace purah { namespace debug {

    const std::map<nds::ASTNodeType, std::string> nodeType_to_string {
        {nds::SimpleAST,"SimpleAST"},
        {nds::NewVarNodeType,"NewVarNodeType"},
        {nds::FunctionExprType,"FunctionExprType"},
        {nds::IdentifierExprType,"IdentifierExprType"},
        {nds::TypedIdentifierExprType,"TypedIdentifierExprType"},
        {nds::ClassObjectExprType,"ClassObjectExprType"},
        {nds::IntExprType,"IntExprType"},
        {nds::FloatExprType,"FloatExprType"},
        {nds::BoolExprType,"BoolExprType"},
        {nds::StringExprType,"StringExprType"},
        {nds::BinaryExprType,"BinaryExprType"},
        {nds::CallExprType,"CallExprType"},
        {nds::ReturnExprType,"ReturnExprType"},
        {nds::IfExprType,"IfExprType"},
        {nds::COUTExprType,"COUTExprType"}
    };

} }