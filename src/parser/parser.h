//
// Created by Morgan Okumu.
//

#ifndef VENOM_PARSER_H
#define VENOM_PARSER_H


#include "ast.h"
#include "../lexer/lexer.h"

namespace parser {

    class Parser {

        public:
            explicit Parser(lexer::Lexer*);
            Parser(lexer::Lexer*, unsigned int);
            ASTProgramNode* parse_program();
            ASTExprNode* parse_expression();  // public for repl
            std::string removelastSpace(std::string str) {
                return removewhiteEnd(str);
            }

        private:

            lexer::Lexer* lex;
            lexer::Token current_token;
            lexer::Token next_token;
            std::string removewhiteEnd(std::string str);
            void consume_token();
			 // Iclude Node
			 ASTIncludeNode*             parse_include();
            // Statement Nodes
            ASTStatementNode*             parse_statement();
            ASTStatementNode*             parse_identifier_statement();
            ASTDeclarationNode*           parse_declaration_statement();
            ASTAssignmentNode*            parse_assignment_statement();
            ASTPrintNode*                 parse_print_statement();
            ASTReturnNode*                parse_return_statement();
            ASTBlockNode*                 parse_block();
            ASTIfNode*                    parse_if_statement();
            ASTWhileNode*                 parse_while_statement();
            ASTFunctionDefinitionNode*    parse_function_definition();
            ASTAppendNode*                parse_append_statement();

            // Expression Nodes
            ASTExprNode*               parse_simple_expression();
            ASTExprNode*               parse_term();
            ASTExprNode*               parse_factor();
            ASTFunctionCallNode*       parse_function_call();


            // Parse Types and parameters
            TYPE parse_type(std::string&);
            std::vector<ASTExprNode*> *parse_actual_params();
            std::pair<std::string, TYPE>* parse_formal_param();
            
            // Global controllers
            bool assign_without_set_token = false;
            std::string period_based_identifier;
            int period_based_line_number;

    };

}

#endif //VENOM_PARSER_H
