//
// Copyright Lukec
// Created by Morgan Okumu on 11/11/22.
//

#ifndef VENOM_SEMANTIC_ANALYSIS_H
#define VENOM_SEMANTIC_ANALYSIS_H

#include <map>
#include <vector>
#include <stack>
#include <bits/stdc++.h>
#include "../parser/ast.h"

namespace visitor {

    void file_include(std::string);
    class SemanticScope {
    public:
        bool already_declared(std::string);
        bool already_declared(std::string, std::vector<parser::TYPE>);
        void declare(std::string, parser::TYPE, unsigned int,  unsigned long int);
        void declare(std::string, parser::TYPE, std::vector<parser::TYPE>, unsigned int);
        parser::TYPE type(std::string);
        parser::TYPE type(std::string, std::vector<parser::TYPE>);
        unsigned int declaration_line(std::string);
        unsigned int declaration_line(std::string, std::vector<parser::TYPE>);

        std::vector<std::pair<std::string, std::string>> function_list();
        std::map<std::string, 
                 std::pair<parser::TYPE, unsigned long int>> array_size_table;

    private:
        std::map<std::string,
                 std::pair<parser::TYPE,
                           unsigned int>> variable_symbol_table;

        std::multimap<std::string,
                      std::tuple<parser::TYPE,
                                 std::vector<parser::TYPE>,
                                 unsigned int>> function_symbol_table;

    };

    class SemanticAnalyser : Visitor {
    public:
        SemanticAnalyser();
        SemanticAnalyser(SemanticScope*);
        ~SemanticAnalyser();

        void visit(parser::ASTProgramNode*) override;
        void visit(parser::ASTDeclarationNode*) override;
        void visit(parser::ASTAssignmentNode*) override;
        void visit(parser::ASTPrintNode*) override;
        void visit(parser::ASTIncludeNode*) override;
        void visit(parser::ASTReturnNode*) override;
        void visit(parser::ASTBlockNode*) override;
        void visit(parser::ASTIfNode*) override;
        void visit(parser::ASTWhileNode*) override;
        void visit(parser::ASTFunctionDefinitionNode*) override;
        void visit(parser::ASTLiteralNode<long int>*) override;
        void visit(parser::ASTLiteralNode<long double>*) override;
        void visit(parser::ASTLiteralNode<bool>*) override;
        void visit(parser::ASTLiteralNode<std::string>*) override;
        void visit(parser::ASTLiteralNode<long int*>*) override;
        void visit(parser::ASTLiteralNode<long double*>*) override;
        void visit(parser::ASTLiteralNode<bool*>*) override;
        void visit(parser::ASTLiteralNode<std::string*>*) override;
        void visit(parser::ASTBinaryExprNode*) override;
        void visit(parser::ASTIdentifierNode*) override;
        void visit(parser::ASTUnaryExprNode*) override;
        void visit(parser::ASTFunctionCallNode*) override;
        void visit(parser::ASTAppendNode * ) override;

    private:
        std::vector<SemanticScope*> scopes;
        std::stack<parser::TYPE> functions;
        parser::TYPE current_expression_type;
        unsigned long int current_array_size;
        std::vector<std::pair<std::string, parser::TYPE>> current_function_parameters;
        bool returns(parser::ASTStatementNode*);
    };

    std::string type_str(parser::TYPE);
}

#endif //VENOM_SEMANTIC_ANALYSIS_H
