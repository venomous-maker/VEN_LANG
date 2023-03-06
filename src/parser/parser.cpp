//
// @author Morgan Okumu
//

#include <iostream>
#include "parser.h"
#include "../inclussion.h"
using namespace std;
using namespace parser;

Parser::Parser(lexer::Lexer* lex) : lex(lex) {
    current_token = lex->next_token();
    std::string temp = "";
    temp  = removewhiteEnd(current_token.value);
    current_token.value = temp;
    next_token = lex->next_token();
    temp = removewhiteEnd(next_token.value);
    next_token.value = temp;
}

Parser::Parser(lexer::Lexer* lex, unsigned int tokens) : lex(lex) {
    next_token = lex->next_token();
    for(unsigned int i = 0; i < tokens; i++)
        consume_token();
}
std::string parser::Parser::removewhiteEnd(std::string str)
{
    std::string::iterator it = str.end();
    it--;

  while(*it == ' ') {
    str.erase(it);
    it = str.end();
    it--;
  }
  return str;
}

void Parser::consume_token() {
    current_token = next_token;
    std::string temp = "";
   temp  = removewhiteEnd(current_token.value);
   current_token.value = temp;
    next_token = lex->next_token();
    temp = removewhiteEnd(next_token.value);
    next_token.value = temp;
    
}

ASTProgramNode* Parser::parse_program() {

    auto statements = new std::vector<ASTNode*>;

    while(current_token.type != lexer::TOK_EOF){
        statements->push_back(parse_statement());
        consume_token();
    }

    return new ASTProgramNode(*statements);
}

ASTStatementNode* Parser::parse_statement() {
    switch(current_token.type){

        /*case lexer::TOK_VAR:
            return parse_declaration_statement();*/

        case lexer::TOK_SET:
            return parse_assignment_statement();

        case lexer::TOK_PRINT:
            return parse_print_statement();

        case lexer::TOK_IF:
            return parse_if_statement();

        case lexer::TOK_WHILE:
            return parse_while_statement();

        case lexer::TOK_RETURN:
            return parse_return_statement();

        case lexer::TOK_DEF:
            return parse_function_definition();

        case lexer::TOK_LEFT_CURLY:
            return parse_block();

        case lexer::TOK_INCLUDE:
            return parse_include();
		case lexer::TOK_REAL_TYPE:
			return parse_declaration_statement();
		case lexer::TOK_INT_TYPE:
			return parse_declaration_statement();
		case lexer::TOK_STRING_TYPE:
			return parse_declaration_statement();
		case lexer::TOK_BOOL_TYPE:
			return parse_declaration_statement();
        default:
            throw std::runtime_error("Invalid statement starting with '" +
                                     current_token.value
                                     + "' encountered on line " +
                                     std::to_string(current_token.line_number) + ".");
    }
}
/*
* @std declaration as of Feb 2023 
* var_type var_name = val;
*/
ASTDeclarationNode* Parser::parse_declaration_statement() {

    // Node attributes
    TYPE type;
    std::string identifier;
    ASTExprNode* expr;
    ASTExprNode** array_expr = nullptr;
    ASTExprNode** temp_array_expr;
    unsigned int line_number;
    bool is_array = false;
    // Eat type
    //consume_token();
	identifier = current_token.value;
    type = parse_type(identifier);
    
    // Determine line number
    line_number = current_token.line_number;
    // Consume identifier
    consume_token();
    if(current_token.type != lexer::TOK_IDENTIFIER)
        throw std::runtime_error("Expected variable name after '"+identifier+"' on line "
                                 + std::to_string(current_token.line_number) + ".");
    identifier = current_token.value;

    consume_token();
    // During the assignment an extra space might be parsed handle it in the operator
    if(current_token.type != lexer::TOK_COLON && current_token.type != lexer::TOK_EQUALS && current_token.value != "=" && current_token.value != ":")
        throw std::runtime_error("Expected ':' or '=' after '" + identifier + "' on line "
                                 + std::to_string(current_token.line_number) + ".");
    // Outdated feature
    /*consume_token();
    if(current_token.type != lexer::TOK_COLON && current_token.type != lexer::TOK_EQUALS && current_token.value != "= " && current_token.value != ": ")
        throw std::runtime_error("Expected assignment operator '=' or ':' for " + identifier + " on line "
                                 + std::to_string(current_token.line_number) + ".");*/
    if (next_token.value == "[") {
        is_array = true;
        // Eat [
        consume_token();
        long int size = 0;
        // Allocate memory for first element if ] is not found
        if (next_token.value != "]") {
            array_expr = (ASTExprNode**)malloc(sizeof(ASTExprNode*) * (size+1));
        }
        // Consume elements
        while(next_token.value != "]") {
            array_expr[size] = parse_expression();
            // Check for comma
            //std::cout << next_token.type << " " << next_token.value<<  " " << lexer::TOK_RIGHT_SQUARE_BRACKET << std::endl;
            if(next_token.type != lexer::TOK_COMMA && next_token.value != "]")
                throw std::runtime_error("Expected ',' or ']' after " + current_token.value +  " on line "
                                 + std::to_string(current_token.line_number) + ".");
            if (next_token.type == lexer::TOK_COMMA) {
                consume_token();
                // Swap and allocate new memory for the elements
                temp_array_expr = array_expr;
                ++size;
                array_expr = (ASTExprNode**)malloc(sizeof(ASTExprNode*) * (size+1));
                unsigned int iter = 0;
                while (iter < size) {
                    array_expr[iter] = temp_array_expr[iter];
                    ++iter;
                }
            }
        }
        // Eat ]
        consume_token();
        //  Free temporary memory for array
        //if (temp_array_expr) free(temp_array_expr);
        std::string identifier_ = current_token.value;
        consume_token();
        if(current_token.type != lexer::TOK_SEMICOLON)
            throw std::runtime_error("Expected ';' after assignment of " + identifier_ + " on line "
                                    + std::to_string(current_token.line_number) + ".");
        // Create ASTExpressionNode to return
        return new ASTDeclarationNode(type, identifier, array_expr, line_number, is_array, (array_expr == nullptr)  ? 0:size+1);
    }
    // If not array
    else{
        // Parse the right hand side
        expr = parse_expression();
        global::global_simple_expr = expr;
        consume_token();
        if(current_token.type != lexer::TOK_SEMICOLON)
            throw std::runtime_error("Expected ';' after assignment of " + identifier + " on line "
                                    + std::to_string(current_token.line_number) + ".");

        // Create ASTExpressionNode to return
        return new ASTDeclarationNode(type, identifier, expr, line_number, is_array);
    }
}

ASTAssignmentNode* Parser::parse_assignment_statement() {

    // Node attributes
    std::string identifier;
    ASTExprNode* expr;
    bool f_pos_only = true;
    ASTExprNode** array_expr = nullptr;
    ASTExprNode** temp_array_expr;
    ASTExprNode* first_value = nullptr;
    ASTExprNode* last_value = nullptr;
    bool is_array = false;
    // Determine line number
    unsigned int line_number = current_token.line_number;
	identifier = current_token.value;
    consume_token();
    if(current_token.type != lexer::TOK_IDENTIFIER)
        throw std::runtime_error("Expected variable name after '"+identifier+"' on line "
                                 + std::to_string(current_token.line_number) + ".");
    identifier = current_token.value;
    
    if ( next_token.value == "[") {
        consume_token();
        long int size = 0;
        is_array = true;
        if (next_token.value != "]") {
            //  Eat expression
            //consume_token();
            first_value = parse_expression();
            // Check if : is present
            if (next_token.type == lexer::TOK_COLON) {
                // Consume :
                consume_token();
                // Get last index expr
                last_value = parse_expression();
            }
            
            if (next_token.value != "]")
                throw std::runtime_error("Expected ] after '"+next_token.value+"' on line "
                                 + std::to_string(next_token.line_number) + ".");
            // Eat ]
            else consume_token();
            
            // 
            if(next_token.type != lexer::TOK_COLON && next_token.type != lexer::TOK_EQUALS && next_token.value != "="/* Some chars maybe taken more than =*/)
                throw std::runtime_error("Expected assignment operator '=' or ':' after " + current_token.value + " on line "
                                 + std::to_string(current_token.line_number) + ".");
            // Eat equal token
            consume_token();
            if (last_value ==  nullptr) {
                // allocate memory for expression
                size = 1;
                array_expr = (ASTExprNode**)calloc(size, sizeof(ASTExprNode*)); 
                array_expr[0] = parse_expression();
            }
            else{
                if (next_token.value == "[") {
                    // Eat [
                    consume_token();
                    // Allocate memory for first element if ] is not found
                    if (next_token.value != "]") {
                        array_expr = (ASTExprNode**)calloc(size+1, sizeof(ASTExprNode*));
                    }
                    // Consume elements
                    while(next_token.value != "]") {
                        array_expr[size] = parse_expression();
                        // Check for comma
                        //std::cout << next_token.type << " " << next_token.value<<  " " << lexer::TOK_RIGHT_SQUARE_BRACKET << std::endl;
                        if(next_token.type != lexer::TOK_COMMA && next_token.value != "]")
                            throw std::runtime_error("Expected ',' or ']' after " + current_token.value +  " on line "
                                            + std::to_string(current_token.line_number) + ".");
                        if (next_token.type == lexer::TOK_COMMA) {
                            consume_token();
                            // Swap and allocate new memory for the elements
                            temp_array_expr = array_expr;
                            ++size;
                            array_expr = (ASTExprNode**)calloc(size+1, sizeof(ASTExprNode*));
                            unsigned int iter = 0;
                            while (iter < size) {
                                array_expr[iter] = temp_array_expr[iter];
                                ++iter;
                            }
                        }
                    }
                    // Eat ]
                    consume_token();
                }
            }
            if(next_token.type != lexer::TOK_SEMICOLON)
                throw std::runtime_error("Expected ';' after " + current_token.value + " on line "
                                        + std::to_string(current_token.line_number) + ".");
            //  Eat ;
            consume_token();
            return new ASTAssignmentNode(identifier, array_expr, line_number, is_array, (array_expr == nullptr)  ? 0:size, first_value, last_value, false);
        }

    }

    consume_token();
    if(current_token.type != lexer::TOK_COLON && current_token.type != lexer::TOK_EQUALS && current_token.value != "="/* Some chars maybe taken more than =*/)
        throw std::runtime_error("Expected assignment operator '=' or ':' after " + identifier + " on line "
                                 + std::to_string(current_token.line_number) + ".");
    
    if (next_token.value == "[") {
        is_array = true;
        // Eat [
        consume_token();
        long int size = 0;
        // Allocate memory for first element if ] is not found
        if (next_token.value != "]") {
            array_expr = (ASTExprNode**)calloc(size+1, sizeof(ASTExprNode*));
        }
        // Consume elements
        while(next_token.value != "]") {
            array_expr[size] = parse_expression();
            // Check for comma
            //std::cout << next_token.type << " " << next_token.value<<  " " << lexer::TOK_RIGHT_SQUARE_BRACKET << std::endl;
            if(next_token.type != lexer::TOK_COMMA && next_token.value != "]")
                throw std::runtime_error("Expected ',' or ']' after " + current_token.value +  " on line "
                                 + std::to_string(current_token.line_number) + ".");
            if (next_token.type == lexer::TOK_COMMA) {
                consume_token();
                // Swap and allocate new memory for the elements
                temp_array_expr = array_expr;
                ++size;
                array_expr = (ASTExprNode**)calloc(size+1, sizeof(ASTExprNode*));
                unsigned int iter = 0;
                while (iter < size) {
                    array_expr[iter] = temp_array_expr[iter];
                    ++iter;
                }
            }
        }
        // Eat ]
        consume_token();
        //  Free temporary memory for array
        //if (temp_array_expr) free(temp_array_expr);
        std::string identifier_ = current_token.value;
        consume_token();
        if(current_token.type != lexer::TOK_SEMICOLON)
            throw std::runtime_error("Expected ';' after assignment of " + identifier_ + " on line "
                                    + std::to_string(current_token.line_number) + ".");
        // Create ASTExpressionNode to return
        if (last_value !=  nullptr ||  first_value != nullptr) {
            return new ASTAssignmentNode(identifier, array_expr, line_number, is_array, (array_expr == nullptr)  ? 0:size+1, first_value, last_value, false);
        }
        else
        return new ASTAssignmentNode(identifier, array_expr, line_number, is_array, (array_expr == nullptr)  ? 0:size+1);
    }
    // Parse the right hand side
    expr = parse_expression();
    global::global_simple_expr = expr;

    consume_token();
    if(current_token.type != lexer::TOK_SEMICOLON)
        throw std::runtime_error("Expected ';' after assignment of " + identifier + " on line "
                                 + std::to_string(current_token.line_number) + ".");

    return new ASTAssignmentNode(identifier, expr, line_number, false);
}

ASTPrintNode* Parser::parse_print_statement() {

    // Determine line number
    global::global_print_val = removewhiteEnd(current_token.value);
    unsigned int line_number = current_token.line_number;
	std::string identifier = current_token.value;
	consume_token();
	// Make sure it's a '('
    if(current_token.type != lexer::TOK_LEFT_BRACKET)
        throw std::runtime_error("Expected '(' after "+ identifier +" on line "
                                 + std::to_string(current_token.line_number) + ".");
    // Get expression to print
    ASTExprNode* expr = parse_expression();
    global::global_simple_expr = expr;
    
	consume_token();
	// Make sure it's a ')'
    if(current_token.type != lexer::TOK_RIGHT_BRACKET)
        throw std::runtime_error("Expected ')' after output expression on line "+ 
                                    std::to_string(current_token.line_number) + ".");
    identifier = current_token.value;
    // Consume ';' token
    consume_token();

    // Make sure it's a ';'
    if(current_token.type != lexer::TOK_SEMICOLON)
        throw std::runtime_error("Expected ';' after '"+identifier+"' on line "
                                 + std::to_string(current_token.line_number) + ".");

    // Return return node
    return new ASTPrintNode(expr, line_number);
}

ASTReturnNode* Parser::parse_return_statement() {

    // Determine line number
    unsigned int line_number = current_token.line_number;

    // Get expression to return
    ASTExprNode* expr = parse_expression();
    global::global_simple_expr = expr;

    // Consume ';' token
    consume_token();

    // Make sure it's a ';'
    if(current_token.type != lexer::TOK_SEMICOLON)
        throw std::runtime_error("Expected ';' after return statement on line "
                                 + std::to_string(current_token.line_number) + ".");

    // Return return node
    return new ASTReturnNode(expr, line_number);
}

//update by venom
ASTIncludeNode* Parser::parse_include(){
	 // Node attributes
    std::string identifier = current_token.value;
    unsigned int line_number = current_token.line_number;

    // Determine line number
    // Consume identifier
    consume_token();
    if(current_token.type != lexer::TOK_IDENTIFIER)
        throw std::runtime_error("Expected file name after "+identifier+" on line "
                                 + std::to_string(current_token.line_number) + ".");
    std:string file_name = current_token.value;
    consume_token();
    // make sure there is a ';'
    if(current_token.type != lexer::TOK_SEMICOLON)
        throw std::runtime_error("Expected ';'after "+file_name+" on line "
                                 + std::to_string(current_token.line_number) + ".");
    //return node
    return new ASTIncludeNode(file_name,line_number);
}

ASTBlockNode* Parser::parse_block() {

    auto statements = new std::vector<ASTStatementNode*>;

    // Determine line number
    unsigned int line_number = current_token.line_number;

    // Current token is '{', consume first token of statement
    consume_token();

    // While not reached end of block or end of file
    while(current_token.type != lexer::TOK_RIGHT_CURLY &&
          current_token.type != lexer::TOK_ERROR &&
          current_token.type != lexer::TOK_EOF){

        // Parse the statement
        statements->push_back(parse_statement());

        // Consume first token of next statement
        consume_token();
    }

    // If block ended by '}', return block
    if(current_token.type == lexer::TOK_RIGHT_CURLY)
        return new ASTBlockNode(*statements, line_number);

    // Otherwise the user left the block open
    else throw std::runtime_error("Reached end of file while parsing."
                                  " Mismatched scopes.");
}

ASTIfNode* Parser::parse_if_statement() {

    //Node attributes
    ASTExprNode* condition;
    ASTBlockNode* if_block;
    ASTExprNode** else_condition = nullptr;
    ASTBlockNode** else_if_block = nullptr;
    ASTExprNode** temp_else_condition = nullptr;
    ASTBlockNode** temp_else_if_block = nullptr;
    unsigned int line_number = current_token.line_number;

    // Consume '('
    consume_token();
    if(current_token.type != lexer::TOK_LEFT_BRACKET)
        throw std::runtime_error("Expected '(' after 'if' on line " +
                                 std::to_string(current_token.line_number) + ".");

    // Parse the expression
    condition = parse_expression();
    global::global_simple_expr = condition;

    // Consume ')'
    consume_token();
    if(current_token.type != lexer::TOK_RIGHT_BRACKET)
        throw std::runtime_error("Expected ')' after if-condition on line " +
                                 std::to_string(current_token.line_number) + ".");

    // Consume '{'
    consume_token();
    if(current_token.type != lexer::TOK_LEFT_CURLY)
        throw std::runtime_error("Expected '{' after if-condition on line " +
                                 std::to_string(current_token.line_number) + ".");

    // Consume if-block and '}'
    if_block = parse_block();
    int i = 0;
    
    // Lookahead whether there is an else
    if(next_token.type != lexer::TOK_ELSE)
        return new ASTIfNode(condition, if_block, line_number);

    // Otherwise, consume the else
    consume_token();
    /**
     *@memory_allocation if else if statement is foreseen.
     */
    if(next_token.type == lexer::TOK_IF){
        else_condition = (ASTExprNode**)malloc(sizeof(ASTExprNode*)*(i+1));
        else_if_block = (ASTBlockNode**)malloc(sizeof(ASTBlockNode*)*(i+1));
    }
    
    while(next_token.type == lexer::TOK_IF){
        
        consume_token();
        // Consume '('
        consume_token();
        if(current_token.type != lexer::TOK_LEFT_BRACKET)
            throw std::runtime_error("Expected '(' after 'if' on line " +
                                    std::to_string(current_token.line_number) + ".");

        // Parse the expression
        else_condition[i] = parse_expression();
        global::global_simple_expr = else_condition[i];

        // Consume ')'
        consume_token();
        if(current_token.type != lexer::TOK_RIGHT_BRACKET)
            throw std::runtime_error("Expected ')' after if-condition on line " +
                                    std::to_string(current_token.line_number) + ".");

        // Consume '{'
        consume_token();
        if(current_token.type != lexer::TOK_LEFT_CURLY)
            throw std::runtime_error("Expected '{' after if-condition on line " +
                                    std::to_string(current_token.line_number) + ".");

        // Consume if-block and '}'
        else_if_block[i] = parse_block();

        // Lookahead whether there is an else
        if(next_token.type != lexer::TOK_ELSE)
            return new ASTIfNode(condition, if_block, line_number, else_if_block, else_condition);
        else  // Otherwise, consume the else
            consume_token();
        // foresee if
        if(next_token.type == lexer::TOK_IF){
            i = i + 1;
            // Memory swapping
            // Allocate memory size for temp = real
            temp_else_condition = (ASTExprNode**)malloc(sizeof(ASTExprNode*)*(i-1));
            temp_else_if_block = (ASTBlockNode**)malloc(sizeof(ASTBlockNode*)*(i-1));
            // Assign
            temp_else_condition = else_condition;
            temp_else_if_block = else_if_block;
            // Allocate new memory for block and condition pointers
            else_condition = (ASTExprNode**)malloc(sizeof(ASTExprNode*)*(i));
            else_if_block = (ASTBlockNode**)malloc(sizeof(ASTBlockNode*)*(i));
            int j = 0;
            // Swapp
            while(temp_else_condition[j]){
                else_condition[j] = temp_else_condition[j];
                else_if_block[j] = temp_else_if_block[j];
                ++j;
            }
            
        }
    }
    // Releasing temporary memory allocation
    if(temp_else_condition){
        free(temp_else_condition);
        free(temp_else_if_block);
    }
    // Consume '{' after else
    consume_token();
    if(current_token.type != lexer::TOK_LEFT_CURLY)
        throw std::runtime_error("Expected '{' after else on line " +
                                 std::to_string(current_token.line_number) + ".");

    // Parse else-block and '}'
    ASTBlockNode* else_block = parse_block();

    // Return if node
    return new ASTIfNode(condition, if_block, line_number, else_if_block, else_condition, else_block);
}

ASTWhileNode* Parser::parse_while_statement() {

    //Node attributes
    ASTExprNode* condition;
    ASTBlockNode* block;
    unsigned int line_number = current_token.line_number;

    // Consume '('
    consume_token();
    if(current_token.type != lexer::TOK_LEFT_BRACKET)
        throw std::runtime_error("Expected '(' after 'while' on line " +
                                 std::to_string(current_token.line_number) + ".");

    // Parse the expression
    condition = parse_expression();
    global::global_simple_expr = condition;

    // Consume ')'
    consume_token();
    if(current_token.type != lexer::TOK_RIGHT_BRACKET)
        throw std::runtime_error("Expected ')' after while-condition on line " +
                                 std::to_string(current_token.line_number) + ".");

    // Consume '{'
    consume_token();
    if(current_token.type != lexer::TOK_LEFT_CURLY)
        throw std::runtime_error("Expected '{' after while-condition on line " +
                                 std::to_string(current_token.line_number) + ".");

    // Consume while-block and '}'
    block = parse_block();

    // Return while node
    return new ASTWhileNode(condition, block, line_number);
}

// STD VENOM func definition func return_type func_name(args){block}

ASTFunctionDefinitionNode* Parser::parse_function_definition() {

    // Node attributes
    std::string identifier;
    std::vector<std::pair<std::string, TYPE>> parameters;
    TYPE type;
    ASTBlockNode* block;
    unsigned int line_number = current_token.line_number;
    // Consume type
    consume_token();
	identifier = current_token.value;
    type = parse_type(identifier);
    // Consume identifier
    consume_token();

    // Make sure it is an identifier
    if(current_token.type != lexer::TOK_IDENTIFIER)
        throw std::runtime_error("Expected function identifier after keyword '"+identifier+"' on line "
                                 + std::to_string(current_token.line_number) + ".");

    identifier = current_token.value;

    // Consume '('
    consume_token();
    if(current_token.type != lexer::TOK_LEFT_BRACKET)
        throw std::runtime_error("Expected '(' after '" + identifier + "' on line "
                                 + std::to_string(current_token.line_number) + ".");

    // Consume ')' or parameters
    consume_token();

    if(current_token.type != lexer::TOK_RIGHT_BRACKET){

        // Parse first parameter
        parameters.push_back(*parse_formal_param());

        // Consume ',' or ')'
        consume_token();

        while(current_token.type == lexer::TOK_COMMA){

            // Consume identifier
            consume_token();

            // Parse parameter
            parameters.push_back(*parse_formal_param());

            // Consume ',' or ')'
            consume_token();
        }

        // Exited while-loop, so token must be ')'
        if(current_token.type != lexer::TOK_RIGHT_BRACKET)
            throw std::runtime_error("Expected ')' or more parameters on line "
                                     + std::to_string(current_token.line_number) + ".");
    }

   /* // Consume ':'
    consume_token();

    if(current_token.type != lexer::TOK_COLON)
        throw std::runtime_error("Expected ':' after ')' on line "
                                 + std::to_string(current_token.line_number) + ".");

*/

    // Consume '{'
    consume_token();

    if(current_token.type != lexer::TOK_LEFT_CURLY)
        throw std::runtime_error("Expected '{' after function '" + identifier +
                                 "' definition on line "
                                 + std::to_string(current_token.line_number) + ".");

    // Parse block
    block = parse_block();

    // Return function definition node
    return new ASTFunctionDefinitionNode(identifier, parameters, type, block, line_number);

}

std::pair<std::string, TYPE>* Parser::parse_formal_param() {

    std::string identifier;
    TYPE type;
	/*This meets venom std argument definition*/
	type = parse_type(identifier);
	
	consume_token();
	
    // Make sure current token is identifier
    if(current_token.type != lexer::TOK_IDENTIFIER)
        throw std::runtime_error("Expected variable name in function definition on line "
                                 + std::to_string(current_token.line_number) + ".");
    identifier = current_token.value;

    /*// Consume ':'
    consume_token();

    if(current_token.type != lexer::TOK_COLON)
        throw std::runtime_error("Expected ':' after '" + identifier + "' on line "
                                 + std::to_string(current_token.line_number) + ".");

    // Consume type
    consume_token();
    type = parse_type(identifier);*/

    return new std::pair<std::string, TYPE>(identifier, type);

};

ASTExprNode* Parser::parse_expression() {
    ASTExprNode *simple_expr = parse_simple_expression();
    unsigned int line_number = current_token.line_number;
    if(next_token.type == lexer::TOK_NOT){
        consume_token();
        //cout << next_token.value << endl;
        if(next_token.value == "=" || removewhiteEnd(next_token.value) == "==")
        {
            consume_token();
            return new ASTBinaryExprNode("!=", simple_expr, parse_expression(), line_number);
        }
        return new ASTUnaryExprNode(current_token.value, parse_expression(), line_number);
    }
    if(next_token.value == "is"){
        consume_token();
        //cout << next_token.value << endl;
        if(next_token.type == lexer::TOK_NOT)
        {
            consume_token();
            return new ASTBinaryExprNode("!=", simple_expr, parse_expression(), line_number);
        }
        return new ASTBinaryExprNode("==", simple_expr, parse_expression(), line_number);
    }

    if(next_token.type == lexer::TOK_RELATIONAL_OP) {
        consume_token();
        return new ASTBinaryExprNode(current_token.value, simple_expr, parse_expression(), line_number);
    }

    return simple_expr;
}

ASTExprNode* Parser::parse_simple_expression() {

    ASTExprNode *term = parse_term();
    unsigned int line_number = current_token.line_number;

    if(next_token.type == lexer::TOK_ADDITIVE_OP) {
        consume_token();
        return new ASTBinaryExprNode(current_token.value, term, parse_simple_expression(), line_number);
    }

    return term;
}

ASTExprNode* Parser::parse_term() {

    ASTExprNode *factor = parse_factor();
    unsigned int line_number = current_token.line_number;

    if(next_token.type == lexer::TOK_MULTIPLICATIVE_OP) {
        consume_token();
        return new ASTBinaryExprNode(current_token.value, factor, parse_term(), line_number);
    }

    return factor;
}

ASTExprNode* Parser::parse_factor() {

    consume_token();

    // Determine line number
    unsigned int line_number = current_token.line_number;

    switch(current_token.type){

        // Literal Cases
        case lexer::TOK_INT:
            return new ASTLiteralNode<long int>(std::stol(current_token.value), line_number);

        case lexer::TOK_REAL:
            return new ASTLiteralNode<long double>(std::stold(current_token.value), line_number);

        case lexer::TOK_BOOL:
            return new ASTLiteralNode<bool>(current_token.value == "true", line_number);

        case lexer::TOK_STRING: {
            // Remove " character from front and end of lexeme
            std::string str = current_token.value.substr(1, current_token.value.size() - 2);

            // Replace \" with quote
            size_t pos = str.find("\\\"");
            while (pos != std::string::npos) {
                // Replace
                str.replace(pos, 2, "\"");
                // Get next occurrence from current position
                pos = str.find("\\\"", pos + 2);
            }

            // Replace \n with newline
            pos = str.find("\\n");
            while (pos != std::string::npos) {
                // Replace
                str.replace(pos, 2, "\n");
                // Get next occurrence from current position
                pos = str.find("\\n", pos + 2);
            }

            // Replace \t with tab
            pos = str.find("\\t");
            while (pos != std::string::npos) {
                // Replace
                str.replace(pos, 2, "\t");
                // Get next occurrence from current position
                pos = str.find("\\t", pos + 2);
            }

            // Replace \b with backslash
            pos = str.find("\\b");
            while (pos != std::string::npos) {
                // Replace
                str.replace(pos, 2, "\\");
                // Get next occurrence from current position
                pos = str.find("\\b", pos + 2);
            }

            return new ASTLiteralNode<std::string>(std::move(str), line_number);
        }

        // Identifier or function call case
        case lexer::TOK_IDENTIFIER:
            if(next_token.type == lexer::TOK_LEFT_BRACKET)
                return parse_function_call();
            else{
                std::string identifier = current_token.value;
                // Handling Arrays
                if (next_token.value == "[") {
                    consume_token();
                    // Get array index
                    ASTExprNode *expr = parse_expression();
                    ASTExprNode *expr_ = nullptr;
                    // Expect ]
                    if (next_token.type == lexer::TOK_COLON) {
                        consume_token();                    // EAT :
                        // parse_expression
                        expr_ = parse_expression();
                    }
                    if (next_token.value != "]")
                        throw std::runtime_error("Expected ']' after "+current_token.value+" on line "
                                         + std::to_string(current_token.line_number) + ".");
                    else{                                   // Eat ]
                        consume_token();
                        return new ASTIdentifierNode(identifier, line_number,  expr,  expr_);
                    }
                }
                return new ASTIdentifierNode(current_token.value, line_number);
            }

        // Subexpression case
        case lexer::TOK_LEFT_BRACKET: {
            ASTExprNode *sub_expr = parse_expression();
            consume_token();
            if (current_token.type != lexer::TOK_RIGHT_BRACKET)
                throw std::runtime_error("Expected ')' after expression on line "
                                         + std::to_string(current_token.line_number) + ".");
            return sub_expr;
        }

        // Unary expression case
        case lexer::TOK_ADDITIVE_OP:
        case lexer::TOK_NOT:
            return new ASTUnaryExprNode(current_token.value, parse_expression(), line_number);

        default:
            throw std::runtime_error("Expected expression on line "
                                     + std::to_string(current_token.line_number) + ".");

    }

}

ASTFunctionCallNode* Parser::parse_function_call() {
    // current token is the function identifier
    std::string identifier = current_token.value;
    auto *parameters = new std::vector<ASTExprNode*>;
    unsigned int line_number = current_token.line_number;

    consume_token();
    if(current_token.type != lexer::TOK_LEFT_BRACKET)
        throw std::runtime_error("Expected '(' on line "
                                 + std::to_string(current_token.line_number) + ".");

    // If next token is not right bracket, we have parameters
    if(next_token.type != lexer::TOK_RIGHT_BRACKET) {
        parameters = parse_actual_params();
    } else
        // Consume ')'
        consume_token();

    // Ensure right close bracket after fetching parameters
    if(current_token.type != lexer::TOK_RIGHT_BRACKET)
        throw std::runtime_error("Expected ')' on line "
                                 + std::to_string(current_token.line_number)
                                 + " after function parameters.");

    return new ASTFunctionCallNode(identifier, *parameters, line_number);
}

std::vector<ASTExprNode*>* Parser::parse_actual_params() {

    auto parameters = new std::vector<ASTExprNode*>;

    parameters->push_back(parse_expression());
    consume_token();

    // If there are more
    while(current_token.type == lexer::TOK_COMMA) {
        parameters->push_back(parse_expression());
        consume_token();
    }

    return parameters;
}
/*@update*/
TYPE Parser::parse_type(std::string& identifier) {
    switch(current_token.type){
        case lexer::TOK_INT_TYPE:
            return INT;

        case lexer::TOK_REAL_TYPE:
            return REAL;

        case lexer::TOK_BOOL_TYPE:
            return BOOL;

        case lexer ::TOK_STRING_TYPE:
            return STRING;

        default:
            throw std::runtime_error("Expected type for '" + identifier + "' before '"+identifier+"' on line "
                                     + std::to_string(current_token.line_number) + ".");
    }
}
