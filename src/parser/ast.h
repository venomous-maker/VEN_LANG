//
// Created by Morgan Okumu @Lukec
//

#ifndef VENOM_AST_H
#define VENOM_AST_H

#include <string>
#include <vector>
#include "../visitor/visitor.h"

namespace parser {

    // TypeZ
    enum TYPE {INT, REAL, BOOL, STRING, INT_ARR, REAL_ARR, BOOL_ARR,  STRING_ARR};

    // Abstract Nodes
    class ASTNode {
    public:
        virtual void accept(visitor::Visitor*) = 0;
    };

    class ASTStatementNode : public ASTNode {
    public:
        void accept(visitor::Visitor*) override = 0;
    };

    class ASTExprNode : public ASTNode {
    public:
        void accept(visitor::Visitor*) override = 0;
    };

    // Statement Nodes
    class ASTProgramNode : public ASTNode {
    public:
        explicit ASTProgramNode(std::vector<ASTNode*>);
        std::vector<ASTNode*> statements;
        void accept(visitor::Visitor*) override;
    };

    class ASTDeclarationNode : public ASTStatementNode {
    public:
        ASTDeclarationNode(TYPE, std::string, ASTExprNode*, unsigned int,  bool);
        ASTDeclarationNode(TYPE, std::string, ASTExprNode**, unsigned int, bool, unsigned long int);
        TYPE type;
        std::string identifier;
        ASTExprNode *expr;
        ASTExprNode **array_expr;
        unsigned int line_number;
        bool is_array;
        unsigned long int array_size;
        void accept(visitor::Visitor*) override;
    };
	class ASTIncludeNode : public ASTStatementNode {
		public:
			ASTIncludeNode(std::string , unsigned int);
			std::string identifier;
			std::string file_name;
			unsigned int line_number;
			void accept(visitor::Visitor*) override;
	};
    class ASTAssignmentNode : public ASTStatementNode {
    public:
        ASTAssignmentNode(std::string, ASTExprNode* , unsigned int , bool);
        ASTAssignmentNode(std::string, ASTExprNode** , unsigned int , bool, unsigned long int);
        ASTAssignmentNode(std::string, ASTExprNode** , unsigned int , bool, long unsigned int, unsigned int, unsigned int, bool);
        std::string identifier;
        ASTExprNode *expr;
        ASTExprNode **array_expr;
        unsigned int line_number;
        bool is_array;
        unsigned long int array_size;
        unsigned int first_position;
        unsigned int last_position;
        bool change_range;
        void accept(visitor::Visitor*) override;
    };

    class ASTPrintNode : public ASTStatementNode {
    public:
        ASTPrintNode(ASTExprNode*, unsigned int);
        ASTExprNode *expr;
        unsigned int line_number;
        void accept(visitor::Visitor*) override;
    };

    class ASTReturnNode : public ASTStatementNode {
    public:
        ASTReturnNode(ASTExprNode*, unsigned int);
        ASTExprNode *expr;
        unsigned int line_number;
        void accept(visitor::Visitor*) override;
    };

    class ASTBlockNode : public ASTStatementNode {
    public:
        ASTBlockNode(std::vector<ASTStatementNode*>, unsigned int);
        std::vector<ASTStatementNode*> statements;
        unsigned int line_number;
        void accept(visitor::Visitor*) override;
    };

    class ASTIfNode : public ASTStatementNode {
    public:
        ASTIfNode(ASTExprNode* condition, ASTBlockNode* if_block, unsigned int line_number, ASTBlockNode** else_if_block = nullptr, ASTExprNode **else_if_conditions = nullptr, ASTBlockNode* else_block = nullptr);
        ASTExprNode *condition;
        ASTExprNode **else_if_conditions;
        ASTBlockNode *if_block;
        ASTBlockNode **else_if_block;
        ASTBlockNode *else_block;
        unsigned int line_number;
        void accept(visitor::Visitor*) override;
    };

    class ASTWhileNode : public ASTStatementNode {
    public:
        ASTWhileNode(ASTExprNode*, ASTBlockNode*, unsigned int);
        ASTExprNode *condition;
        ASTBlockNode *block;
        unsigned int line_number;
        void accept(visitor::Visitor*) override;
    };

    class ASTFunctionDefinitionNode : public ASTStatementNode {
    public:
        ASTFunctionDefinitionNode(std::string, std::vector<std::pair<std::string, TYPE>>,
                                  TYPE, ASTBlockNode*, unsigned int);
        std::string identifier;
        std::vector<std::pair<std::string, TYPE>> parameters;
        std::vector<std::string> variable_names;
        std::vector<TYPE> signature;
        TYPE type;
        ASTBlockNode* block;
        unsigned int line_number;
        void accept(visitor::Visitor*) override;
    };

    // Expression Nodes
    template <typename T>
    class ASTLiteralNode : public ASTExprNode {
    public:
        ASTLiteralNode(T val, unsigned int line_number) : val(val), line_number(line_number) {};
        T val;
        unsigned int line_number;
        void accept(visitor::Visitor*) override;
    };

    class ASTBinaryExprNode : public ASTExprNode {
    public:
        ASTBinaryExprNode(std::string, ASTExprNode*, ASTExprNode*, unsigned int);
        std::string op;
        ASTExprNode *left;
        ASTExprNode *right;
        unsigned int line_number;
        void accept(visitor::Visitor*) override;
    };

    class ASTIdentifierNode : public ASTExprNode {
    public:
        explicit ASTIdentifierNode(std::string, unsigned int);
        std::string identifier;
        unsigned int line_number;
        void accept(visitor::Visitor*) override;
    };

    class ASTUnaryExprNode : public ASTExprNode {
    public:
        ASTUnaryExprNode(std::string, ASTExprNode*, unsigned int);
        std::string unary_op;
        ASTExprNode *expr;
        unsigned int line_number;
        void accept(visitor::Visitor*) override;
    };

    class ASTFunctionCallNode : public ASTExprNode {
    public:
        ASTFunctionCallNode(std::string, std::vector<ASTExprNode*>, unsigned int);
        std::string identifier;
        std::vector<ASTExprNode*> parameters;
        unsigned int line_number;
        void accept(visitor::Visitor*) override;
    };
}


#endif //VENOM_AST_H
