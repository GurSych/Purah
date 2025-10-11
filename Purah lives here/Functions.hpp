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
        FunctionSignature(std::string _n, std::vector<std::string> _a_t) : FunctionSignature(_n, "", _a_t) {}
        FunctionSignature(nds::ASTPtr& node) {
            if(node->nodeType() != nds::FunctionExprType)
                throw excptn::PurahError("InterpreterError: Expected FunctionExpr node for FunctionSignature");
            nds::FunctionExprNode* func_node = static_cast<nds::FunctionExprNode*>(node.get());
            name = func_node->name;
            type = func_node->type;
            for(auto& arg : func_node->args) {
                if(arg->nodeType() != nds::TypedIdentifierExprType)
                    throw excptn::PurahError("InterpreterError: Expected TypedIdentifier node for function argument");
                nds::TypedIdentifierExprNode* arg_node = static_cast<nds::TypedIdentifierExprNode*>(arg.get());
                args_types.push_back(arg_node->type);
                args_names.push_back(arg_node->name);
            }
        }
        std::string name{};
        std::string type{};
        std::vector<std::string> args_types{};
        std::vector<std::string> args_names{};
        bool operator==(const FunctionSignature& other) const {
            return (name == other.name && args_types == other.args_types);
        }
        bool check_all(const FunctionSignature& other) {
            return (name == other.name && type == other.type && args_types == other.args_types);
        }
        std::string to_string(bool full = true) {
            std::string str = name + '\\';
            if(full) str += type; 
            str += '(';
            unsigned int count{};
            for(std::string& arg : args_types) {
                if(count++ != 0) str += ",";
                str += "\\" + arg;
            }
            str += ')';
            return str;
        }
    };

} }

namespace std {
    template <>
    struct hash<fnc::FunctionSignature> {
        size_t operator()(const fnc::FunctionSignature& sign) const {
            size_t hash = 0ull;
            addserv::hash_combine<std::string>(hash,sign.name);
            for(const std::string& arg : sign.args_types)
                addserv::hash_combine<std::string>(hash,arg);
            return hash;
        }
    };
}