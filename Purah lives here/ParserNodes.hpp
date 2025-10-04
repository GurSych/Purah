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
        IdentifierExprType, 
        IntExprType, FloatExprType,
        BoolExprType, StringExprType,
        BinaryExprType, CallExprType, 
        COUTExprType, PrintExprType
    };

    struct ASTNode {
        virtual ~ASTNode() = default;
        virtual ASTNodeType nodeType() { return SimpleAST; }
    };

    using ASTPtr = std::unique_ptr<ASTNode>;

    class NewVarNode: public ASTNode {
        public:
        ASTNodeType nodeType() override { return NewVarNodeType; }
            NewVarNode(const std::string& _t, const std::string& _n, ASTPtr _v) 
                : varType{_t}, name{_n}, value{std::move(_v)} {}
            std::string varType{};
            std::string name{};
            ASTPtr value{};
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
            std::string name{};
            std::vector<ASTPtr> args{};
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

} }