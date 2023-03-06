//
// Created by Morgan Okumu @Lukec
//


#include "ast.h"
#include <utility>
#include <iostream>

using namespace parser;

// Program Node
ASTProgramNode::ASTProgramNode(std::vector<ASTNode*> statements) :
        statements(std::move(statements))
{}


// Statement Nodes
ASTDeclarationNode::ASTDeclarationNode(TYPE type, std::string identifier, ASTExprNode *expr,
                                       unsigned int line_number,  bool is_array) :
    type(type),
    identifier(std::move(identifier)),
    expr(expr),
    array_expr(nullptr), 
    line_number(line_number), 
    is_array(is_array), 
    array_size(0)
{}
ASTDeclarationNode::ASTDeclarationNode(TYPE type, std::string identifier, ASTExprNode **array_expr,
                                       unsigned int line_number,  bool is_array, unsigned long int array_size) :
    type(type),
    identifier(std::move(identifier)),
    expr(nullptr),
    array_expr(array_expr),
    line_number(line_number),
    is_array(is_array), 
    array_size(array_size)
{}
ASTAssignmentNode::ASTAssignmentNode(std::string identifier, ASTExprNode *expr, unsigned int line_number, bool is_array) :
        identifier(std::move(identifier)),
        expr(expr),
        array_expr(nullptr), 
        line_number(line_number), 
        is_array(is_array),
        array_size(0), 
        first_position(nullptr), 
        last_position(nullptr), 
        change_range(false)
{}

ASTAssignmentNode::ASTAssignmentNode(std::string identifier, ASTExprNode **array_expr, unsigned int line_number, bool is_array,
                                        unsigned long int array_size) :
        identifier(std::move(identifier)),
        expr(nullptr),
        array_expr(array_expr), 
        line_number(line_number), 
        is_array(is_array),
        array_size(array_size),
        first_position(nullptr), 
        last_position(nullptr),
        change_range(true)
{}

ASTAssignmentNode::ASTAssignmentNode(std::string identifier, ASTExprNode **array_expr, unsigned int line_number, bool is_array, 
                                    unsigned long int array_size, ASTExprNode* first_position,  ASTExprNode* last_position, bool change_range) :
        identifier(std::move(identifier)),
        expr(nullptr),
        array_expr(array_expr), 
        line_number(line_number), 
        is_array(is_array),
        array_size(array_size), 
        first_position(first_position), 
        last_position(last_position), 
        change_range(change_range)
{}

ASTIncludeNode::ASTIncludeNode(std::string file_name,unsigned int line_number) :
		file_name(std::move(file_name)),
		line_number(line_number)
{}

ASTPrintNode::ASTPrintNode(ASTExprNode *expr, unsigned int line_number) :
        expr(expr),
        line_number(line_number)
{}

ASTReturnNode::ASTReturnNode(ASTExprNode *expr, unsigned int line_number) :
        expr(expr),
        line_number(line_number)
{}

ASTBlockNode::ASTBlockNode(std::vector<ASTStatementNode*> statements, unsigned int line_number) :
        statements(std::move(statements)),
        line_number(line_number)
{}

ASTIfNode::ASTIfNode(ASTExprNode* condition, ASTBlockNode *if_block, unsigned int line_number,ASTBlockNode** else_if_block, ASTExprNode **else_if_conditions, 
                     ASTBlockNode *else_block) :
        condition(condition),
        else_if_conditions(else_if_conditions),
        if_block(if_block),
        else_if_block(else_if_block),
        else_block(else_block), 
        line_number(line_number)
{}

ASTWhileNode::ASTWhileNode(ASTExprNode *condition, ASTBlockNode *block, unsigned int line_number) :
        condition(condition),
        block(block),
        line_number(line_number)
{}

ASTFunctionDefinitionNode::ASTFunctionDefinitionNode(std::string identifier,
                                                     std::vector<std::pair<std::string, TYPE>> parameters,
                                                     TYPE type, ASTBlockNode* block, unsigned int line_number) :
        identifier(std::move(identifier)),
        parameters(std::move(parameters)),
        type(type),
        block(block),
        line_number(line_number)
{
    // Generate signature
    this->signature = std::vector<TYPE>();
    for(auto param : this->parameters) {
        variable_names.push_back(param.first);
        signature.push_back(param.second);
    }
}


// Expression Nodes
ASTBinaryExprNode::ASTBinaryExprNode(std::string op, ASTExprNode *left, ASTExprNode *right,
                                     unsigned int line_number) :
        op(std::move(op)),
        left(left),
        right(right),
        line_number(line_number)
{}

ASTIdentifierNode::ASTIdentifierNode(std::string identifier, unsigned int line_number) :
        identifier(std::move(identifier)),
        line_number(line_number), 
        array_position(nullptr), 
        last_array_position(nullptr)
{}

ASTIdentifierNode::ASTIdentifierNode(std::string identifier, unsigned int line_number, ASTExprNode* array_position,  ASTExprNode* last_array_position) :
        identifier(std::move(identifier)),
        line_number(line_number), 
        array_position(array_position),
        last_array_position(last_array_position)
{}

ASTUnaryExprNode::ASTUnaryExprNode(std::string unary_op, ASTExprNode *expr, unsigned int line_number) :
    unary_op(std::move(unary_op)),
    expr(expr),
    line_number(line_number)
{}

ASTFunctionCallNode::ASTFunctionCallNode(std::string identifier, std::vector<ASTExprNode*> parameters,
                                         unsigned int line_number) :
    identifier(std::move(identifier)),
    parameters(std::move(parameters)),
    line_number(line_number)
{}


// Accept functions for visitors
void ASTBinaryExprNode::accept(visitor::Visitor *v){
    v -> visit(this);
}

namespace parser {

    template<>
    void ASTLiteralNode<long int>::accept(visitor::Visitor *v) {
        v->visit(this);
    }
    
    template<>
    void ASTLiteralNode<long int*>::accept(visitor::Visitor *v) {
        v->visit(this);
    }

    template<>
    void ASTLiteralNode<long double>::accept(visitor::Visitor *v) {
        v->visit(this);
    }
    
     template<>
    void ASTLiteralNode<long double*>::accept(visitor::Visitor *v) {
        v->visit(this);
    }

    template<>
    void ASTLiteralNode<bool>::accept(visitor::Visitor *v) {
        v->visit(this);
    }
    
    template<>
    void ASTLiteralNode<bool*>::accept(visitor::Visitor *v) {
        v->visit(this);
    }

    template<>
    void ASTLiteralNode<std::string>::accept(visitor::Visitor *v) {
        v->visit(this);
    }
    template<>
    void ASTLiteralNode<std::string*>::accept(visitor::Visitor *v) {
        v->visit(this);
    }
}

void ASTFunctionCallNode::accept(visitor::Visitor *v){
    v -> visit(this);
}

void ASTIdentifierNode::accept(visitor::Visitor *v){
    v -> visit(this);
}

void ASTUnaryExprNode::accept(visitor::Visitor *v){
    v -> visit(this);
}

void ASTDeclarationNode::accept(visitor::Visitor *v){
    v -> visit(this);
}

void ASTAssignmentNode::accept(visitor::Visitor *v){
    v -> visit(this);
}

void ASTPrintNode::accept(visitor::Visitor *v){
    v -> visit(this);
}

void ASTReturnNode::accept(visitor::Visitor *v){
    v -> visit(this);
}

void ASTBlockNode::accept(visitor::Visitor *v){
    v -> visit(this);
}

void ASTIfNode::accept(visitor::Visitor *v){
    v -> visit(this);
}

void ASTWhileNode::accept(visitor::Visitor *v){
    v -> visit(this);
}

void ASTFunctionDefinitionNode::accept(visitor::Visitor *v){
    v -> visit(this);
}
void ASTIncludeNode::accept(visitor::Visitor *v){
    v -> visit(this);
}
void ASTProgramNode::accept(visitor::Visitor *v){
    v -> visit(this);
}
