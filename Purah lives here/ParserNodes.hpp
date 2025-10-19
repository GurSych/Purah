#pragma once
#include <stdint.h>
#include <memory>
#include <vector>
#include <string>

#ifdef EOF
    #undef EOF
#endif

#include "Tokens.hpp"

namespace purah { namespace nds {

    enum ASTNodeType {
        SimpleAST, 
        NewVarNodeType,
        FunctionExprType,
        IdentifierExprType,
        TypedIdentifierExprType,
        ClassObjectExprType,
        IntExprType, FloatExprType,
        BoolExprType, StringExprType,
        BinaryExprType,
        CallExprType, ReturnExprType,
        IfExprType,
        COUTExprType, PrintExprType
    };

    struct ASTNode {
        virtual ~ASTNode() = default;
        virtual ASTNodeType nodeType() { return SimpleAST; }
    };

    using ASTPtr = std::shared_ptr<ASTNode>;

    class NewVarNode: public ASTNode {
        public:
        ASTNodeType nodeType() override { return NewVarNodeType; }
            NewVarNode(const std::string& _t, const std::string& _n, ASTPtr _v) 
                : type{_t}, name{_n}, value{std::move(_v)} {}
            std::string type{};
            std::string name{};
            ASTPtr      value{};
    };

    class FunctionExprNode: public ASTNode {
        public:
        ASTNodeType nodeType() override { return FunctionExprType; }
            FunctionExprNode(const std::string& _n, const std::string& _t, 
                std::vector<nds::ASTPtr>&& _args, std::vector<nds::ASTPtr>&& _body) 
                : name{_n}, type{_t}, args{std::move(_args)}, body{std::move(_body)} { }
            std::string name{};
            std::string type{};
            std::vector<nds::ASTPtr> args{};
            std::vector<nds::ASTPtr> body{}; 
    };

    class TypedIdentifierExprNode: public ASTNode {
        public:
        ASTNodeType nodeType() override { return TypedIdentifierExprType; }
            TypedIdentifierExprNode(std::string _n, std::string _t) : name{_n}, type{_t} {}
            std::string name{};
            std::string type{};
    };

    class IntExprNode: public ASTNode {
        public:
        ASTNodeType nodeType() override { return IntExprType; }
            IntExprNode(int64_t _v) : value{_v} {}
            int64_t value{};
    };

    class FloatExprNode: public ASTNode {
        public:
        ASTNodeType nodeType() override { return FloatExprType; }
            FloatExprNode(double _v) : value{_v} {}
            double value{};
    };

    class BoolExprNode: public ASTNode {
        public:
        ASTNodeType nodeType() override { return BoolExprType; }
            BoolExprNode(bool _v) : value{_v} {}
            bool value{};
    };

    class StringExprNode: public ASTNode {
        public:
        ASTNodeType nodeType() override { return StringExprType; }
            StringExprNode(std::string _v) : value{_v} {}
            std::string value{};
    };

    class IdentifierExprNode: public ASTNode {
        public:
        ASTNodeType nodeType() override { return IdentifierExprType; }
            IdentifierExprNode(std::string _n) : name{_n} {}
            std::string name{};
    };

    class ClassObjectExprNode: public ASTNode {
        public:
        ASTNodeType nodeType() override { return ClassObjectExprType; }
            ClassObjectExprNode() { }
            /// Boo!~ ///
    };

    class BinaryExprNode: public ASTNode {
        public:
        ASTNodeType nodeType() override { return BinaryExprType; }
            BinaryExprNode(tkn::TokenType _op, ASTPtr _left, ASTPtr _right) 
                : op{_op}, left{std::move(_left)}, right{std::move(_right)} {}
            tkn::TokenType op{};
            ASTPtr left{};
            ASTPtr right{};
    };

    class CallExprNode: public ASTNode {
        public:
        ASTNodeType nodeType() override { return CallExprType; }
            CallExprNode(const std::string& _n, std::vector<ASTPtr>&& _a_s)
                : name{_n}, args{std::move(_a_s)} {}
            std::string         name{};
            std::vector<ASTPtr> args{};
            bool                authorized{false};
            ASTPtr              function{};
    };

    class ReturnExprNode: public ASTNode {
        public:
        ASTNodeType nodeType() override { return ReturnExprType; }
            ReturnExprNode(ASTPtr _v) : value{std::move(_v)} { }
            ASTPtr value{};
    };

    class IfExprNode: public ASTNode {
        public:
        ASTNodeType nodeType() override { return IfExprType; }
            IfExprNode(nds::ASTPtr _condition, std::vector<nds::ASTPtr>&& _body, nds::ASTPtr _n_i) 
                : condition{std::move(_condition)}, body{std::move(_body)}, next_if{std::move(_n_i)} { }
            nds::ASTPtr condition{};
            std::vector<nds::ASTPtr> body{};
            nds::ASTPtr next_if{};
    };

    class COUTExprNode: public ASTNode {
        public:
        ASTNodeType nodeType() override { return COUTExprType; }
            COUTExprNode(ASTPtr _v) : value{std::move(_v)} { }
            ASTPtr value{};
    };

    class PrintExprNode: public ASTNode {
        public:
        ASTNodeType nodeType() override { return PrintExprType; }
            PrintExprNode(std::vector<ASTPtr>&& _o_a) : out_args{std::move(_o_a)} {}
            std::vector<ASTPtr> out_args{};
    };

    std::string type_token_to_string(const ASTNodeType type) {
            if (type == IntExprType)    return "int";
            if (type == FloatExprType)  return "float";
            if (type == BoolExprType)   return "bool";
            if (type == StringExprType) return "string";
            throw std::invalid_argument("Unsupported node type for type_token_to_string");
        }

} }