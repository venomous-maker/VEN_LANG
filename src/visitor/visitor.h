//
// Copyright Lukec
// Created by Morgan Okumu on 11/11/22.
//

#ifndef VENOM_VISITOR_H
#define VENOM_VISITOR_H

#include <string>

namespace parser {

    class ASTProgramNode;

    class ASTDeclarationNode;
    class ASTAssignmentNode;
    class ASTPrintNode;
    class ASTReturnNode;
    class ASTBlockNode;
    class ASTIfNode;
    class ASTWhileNode;
    class ASTIncludeNode;
    class ASTAppendNode;
    class ASTFunctionDefinitionNode;

    template <typename T> class ASTLiteralNode;
    class ASTBinaryExprNode;
    class ASTIdentifierNode;
    class ASTUnaryExprNode;
    class ASTFunctionCallNode;
}

namespace visitor {

    class Visitor {

    public:

        virtual void visit(parser::ASTProgramNode*) = 0;
        virtual void visit(parser::ASTIncludeNode*) = 0;
        virtual void visit(parser::ASTAppendNode*) = 0;
        virtual void visit(parser::ASTDeclarationNode*) = 0;
        virtual void visit(parser::ASTAssignmentNode*) = 0;
        virtual void visit(parser::ASTPrintNode*) = 0;
        virtual void visit(parser::ASTReturnNode*) = 0;
        virtual void visit(parser::ASTBlockNode*) = 0;
        virtual void visit(parser::ASTIfNode*) = 0;
        virtual void visit(parser::ASTWhileNode*) = 0;
        virtual void visit(parser::ASTFunctionDefinitionNode*) = 0;
        virtual void visit(parser::ASTLiteralNode<long int>*) = 0;
        virtual void visit(parser::ASTLiteralNode<long double>*) = 0;
        virtual void visit(parser::ASTLiteralNode<bool>*) = 0;
        virtual void visit(parser::ASTLiteralNode<std::string>*) = 0;
        virtual void visit(parser::ASTLiteralNode<long int*>*) = 0;
        virtual void visit(parser::ASTLiteralNode<long double*>*) = 0;
        virtual void visit(parser::ASTLiteralNode<bool*>*) = 0;
        virtual void visit(parser::ASTLiteralNode<std::string*>*) = 0;
        virtual void visit(parser::ASTBinaryExprNode*) = 0;
        virtual void visit(parser::ASTIdentifierNode*) = 0;
        virtual void visit(parser::ASTUnaryExprNode*) = 0;
        virtual void visit(parser::ASTFunctionCallNode*) = 0;
    };

}


#endif //VENOM_VISITOR_H
