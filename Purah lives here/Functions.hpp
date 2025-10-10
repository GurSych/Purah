#pragma once
#include <string>
#include <vector>

#include "Exceptions.hpp"
#include "AdditionalServices.hpp"
#include "ParserNodes.hpp"

using namespace purah;

namespace purah { namespace fnc {

    class FunctionSignature {
    public:
        FunctionSignature(std::string _n, std::string _t, std::vector<std::string> _a_t)
            : name{_n}, type{_t}, args_types{_a_t} {}
        FunctionSignature(nds::ASTPtr& node) {
            if(node->nodeType() != nds::FunctionExprType)
                throw excptn::PurahError("InterpreterError: Expected FunctionExpr node for FunctionSignature");
            nds::FunctionExprNode* func_node = static_cast<nds::FunctionExprNode*>(node.get());
            name = func_node->name;
            type = func_node->type;
            for(auto& arg : func_node->args) {
                if(node->nodeType() != nds::TypedIdentifierExprType)
                    throw excptn::PurahError("InterpreterError: Expected TypedIdentifier node for function argument");
                nds::TypedIdentifierExprNode* arg_node = static_cast<nds::TypedIdentifierExprNode*>(node.get());
                args_types.push_back(arg_node->type);
            }
        }
        std::string name{};
        std::string type{};
        std::vector<std::string> args_types{};
        bool operator==(const FunctionSignature& other) const {
            return (name == other.name && type == other.type && args_types == other.args_types);
        }
    };

} }

namespace std {
    template <>
    struct hash<fnc::FunctionSignature> {
        size_t operator()(const fnc::FunctionSignature& sign) const {
            size_t hash = 0ull;
            addserv::hash_combine<std::string>(hash,sign.name);
            addserv::hash_combine<std::string>(hash,sign.type);
            for(const std::string& arg : sign.args_types)
                addserv::hash_combine<std::string>(hash,arg);
            return hash;
        }
    };
}