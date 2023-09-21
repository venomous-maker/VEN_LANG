//
// Created by Morgan Okumu
//

#ifndef VENOM_INTERPRETER_H
#define VENOM_INTERPRETER_H

#include <map>
#include <stack>
#include "visitor.h"
#include "../parser/ast.h"

namespace visitor {

    typedef struct vT {
        vT() : i(0), r(0), b(0), s(""), i_(nullptr), r_(nullptr), b_(nullptr), s_(nullptr) {};
        long int i;
        long double r;
        bool b;
        std::string s;
        long int *i_;
        long double *r_;
        bool* b_;
        std::string *s_;
    } value_t;


    class InterpreterScope {
    public:

        bool already_declared(std::string);
        bool already_declared(std::string, std::vector<parser::TYPE>);
        void declare(std::string, long int);
        void declare(std::string, long double);
        void declare(std::string, bool);
        void declare(std::string, std::string);
        void declare(std::string, long int*, unsigned long int);
        void declare(std::string, long double*, unsigned long int);
        void declare(std::string, bool*, unsigned long int);
        void declare(std::string, std::string*, unsigned long int);
        void declare(std::string, std::vector<parser::TYPE>, std::vector<std::string>,
                parser::ASTBlockNode*);
		void file_include(std::string fileargs);
        parser::TYPE type_of(std::string);
        value_t value_of(std::string);
        std::vector<std::string> variable_names_of(std::string, std::vector<parser::TYPE>);
        parser::ASTBlockNode* block_of(std::string, std::vector<parser::TYPE>);

        std::vector<std::tuple<std::string, std::string, std::string>>  variable_list();
        std::vector<std::tuple<std::string, std::string, std::string*>> array_variable_list(); 
        std::map<std::string, 
                 std::pair<parser::TYPE, unsigned long int>> array_size_table;

    private:
        std::map<std::string,
                 std::pair<parser::TYPE,
                           value_t>> variable_symbol_table;
        std::multimap<std::string,
                      std::tuple<std::vector<parser::TYPE>,
                                 std::vector<std::string>,
                                 parser::ASTBlockNode*>> function_symbol_table;
    };

    class Interpreter : public Visitor {
    public:

        Interpreter();
        Interpreter(InterpreterScope*);
        ~Interpreter();

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

        std::pair<parser::TYPE, value_t> current_expr();
        unsigned long int get_current_array_size(void);

    private:
        std::vector<InterpreterScope*> scopes;
        parser::TYPE current_expression_type;
        value_t current_expression_value;
        unsigned long int current_array_size;
        std::vector<std::string> current_function_parameters;
        std::vector<std::pair<parser::TYPE, value_t>> current_function_arguments;
    };

    std::string type_str(parser::TYPE);
}

#endif //VENOM_INTERPRETER_H
