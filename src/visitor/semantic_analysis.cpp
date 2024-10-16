/**
* @Copyright Lukec
* @Created by Morgan Okumu on 11/11/22.
*/

#include "semantic_analysis.h"
#include "../inclussion.h"
#include <utility>
#include <iostream>

using namespace visitor;

bool SemanticScope::already_declared(std::string identifier) {
    return variable_symbol_table.find(identifier) != variable_symbol_table.end();
}

bool SemanticScope::already_declared(std::string identifier, std::vector<parser::TYPE> signature) {

    auto funcs = function_symbol_table.equal_range(identifier);

    // If key is not present in multimap
    if(std::distance(funcs.first, funcs.second) == 0)
        return false;

    // Check signature for each function in multimap
    for (auto i = funcs.first; i != funcs.second; i++)
        if(std::get<1>(i->second) == signature)
            return true;

    // Function with matching signature not found
    return false;
}

void SemanticScope::declare(std::string identifier, parser::TYPE type, unsigned int line_number, unsigned long int size) {
    switch (type) {
        case parser::INT:
        case parser::REAL:
        case parser::BOOL:
        case parser::STRING:
            variable_symbol_table[identifier] = std::make_pair(type, line_number);
            break;
        case parser::INT_ARR:
        case parser::REAL_ARR:
        case parser::BOOL_ARR:
        case parser::STRING_ARR:
            variable_symbol_table[identifier] = std::make_pair(type, line_number);
            array_size_table[identifier] = std::make_pair(type, size);
            break;
    }
}

void SemanticScope::declare(std::string identifier, parser::TYPE type, std::vector<parser::TYPE> signature,
                    unsigned int line_number) {

    function_symbol_table
            .insert(std::make_pair(identifier, std::make_tuple(type,
                                                               signature,
                                                               line_number)));
}

parser::TYPE SemanticScope::type(std::string identifier) {

    if(already_declared(identifier))
        return variable_symbol_table[identifier].first;

    throw std::runtime_error("Something went wrong when determining the type of '" + identifier + "'.");
}

parser::TYPE SemanticScope::type(std::string identifier, std::vector<parser::TYPE> signature) {

    auto funcs = function_symbol_table.equal_range(identifier);

    // If key is not present in multimap
    if(std::distance(funcs.first, funcs.second) == 0)
        throw std::runtime_error("Something went wrong when determining the type of '" + identifier + "'.");

    // Check signature for each
    for (auto i = funcs.first; i != funcs.second; i++)
        if(std::get<1>(i->second) == signature)
            return std::get<0>(i->second);

    // Function with matching signature not found
    throw std::runtime_error("Something went wrong when determining the type of '" + identifier + "'.");
}

unsigned int SemanticScope::declaration_line(std::string identifier) {

    if(already_declared(identifier))
        return variable_symbol_table[std::move(identifier)].second;

    throw std::runtime_error("Something went wrong when determining the line number of '" + identifier + "'.");
}

unsigned int SemanticScope::declaration_line(std::string identifier, std::vector<parser::TYPE> signature) {

    auto funcs = function_symbol_table.equal_range(identifier);

    // If key is not present in multimap
    if(std::distance(funcs.first, funcs.second) == 0)
        throw std::runtime_error("Something went wrong when determining the line number of '" + identifier + "'.");

    // Check signature for each
    for (auto i = funcs.first; i != funcs.second; i++)
        if(std::get<1>(i->second) == signature)
            return std::get<2>(i->second);

    // Function with matching signature not found
    throw std::runtime_error("Something went wrong when determining the line number of '" + identifier + "'.");
}


std::vector<std::pair<std::string, std::string>> SemanticScope::function_list() {

    std::vector<std::pair<std::string, std::string>> list;

    for(auto func = function_symbol_table.begin(), last = function_symbol_table.end();
        func != last; func = function_symbol_table.upper_bound(func -> first)){

        std::string func_name = func->first + "(";
        bool has_params = false;
        for(auto param : std::get<1>(func -> second)) {
            has_params = true;
            func_name += type_str(param) + ", ";
        }
        func_name.pop_back();   // remove last whitespace
        func_name.pop_back();   // remove last comma
        func_name += ")";

        list.emplace_back(std::make_pair(func_name, type_str(std::get<0>(func->second))));
    }

    return std::move(list);
}


SemanticAnalyser::SemanticAnalyser() {
    // Add global scope
    scopes.push_back(new SemanticScope());
};

SemanticAnalyser::SemanticAnalyser(SemanticScope* global_scope) {
    // Add global scope
    scopes.push_back(global_scope);
};

SemanticAnalyser::~SemanticAnalyser() = default;

void SemanticAnalyser::visit(parser::ASTProgramNode *prog) {

    // For each statement, accept
    for(auto &statement : prog -> statements)
        statement -> accept(this);
}

void SemanticAnalyser::visit(parser::ASTDeclarationNode *decl){

    // Current scope is the scope at the back
    SemanticScope *current_scope = scopes.back();

    // If variable already declared, throw error
    if(current_scope->already_declared(decl->identifier))
        throw std::runtime_error("Variable redeclaration on line " + std::to_string(decl->line_number) + ". '" +
                                  decl->identifier + "' was already declared in this scope on line " +
                                  std::to_string(current_scope->declaration_line(decl->identifier)) + ".");
    // Check if array
    if (decl->is_array) {
        int array_size = 0;
        //if (decl->array_expr == nullptr) throw std::runtime_error("Blah blah");
        while (array_size < decl->array_size && decl->array_expr != nullptr) {
            decl->array_expr[array_size]->accept(this);
            switch (decl->type) {
                case parser::REAL:
                {
                    if (decl -> type != parser::REAL && current_expression_type != parser::INT)
                        throw std::runtime_error("Found " + type_str(current_expression_type) + " on line " +
                                    std::to_string(decl->line_number) + " in definition of '" +
                                    decl -> identifier + "', expected " + type_str(decl->type) + " in all elements.");
                    break;
                }
                default:
                    
                    if (current_expression_type !=  decl->type && decl->type != parser::REAL)
                        throw std::runtime_error("Found " + type_str(current_expression_type) + " on line " +
                                    std::to_string(decl->line_number) + " in definition of '" +
                                    decl -> identifier + "', expected " + type_str(decl->type) + " in all elements.");
            }
            array_size++;
        }
        // allow mismatched type in the case of declaration of int to real
        if(decl -> type == parser::REAL && current_expression_type == parser::INT)
            current_scope->declare(decl->identifier, parser::REAL_ARR, decl->line_number,  decl->array_size);

        // types match
        // Make sure if no elements you assign null space
        else if (decl -> type == current_expression_type || decl->array_expr == nullptr)
        {
            if (decl -> type == parser::INT) current_scope->declare(decl->identifier, parser::INT_ARR, decl->line_number, decl->array_size);
            if (decl -> type == parser::REAL) current_scope->declare(decl->identifier, parser::REAL_ARR, decl->line_number,  decl->array_size);
            if (decl -> type == parser::BOOL) current_scope->declare(decl->identifier, parser::BOOL_ARR, decl->line_number,  decl->array_size);
            if (decl -> type == parser::STRING) current_scope->declare(decl->identifier, parser::STRING_ARR, decl->line_number,  decl->array_size);
        }
        // types don't match
        else
            throw std::runtime_error("Found " + type_str(current_expression_type) + " on line " +
                                    std::to_string(decl->line_number) + " in definition of '" +
                                    decl -> identifier + "', expected " + type_str(decl->type) + " in all elements.");
    }else{
        // Visit the expression to update current type
        decl -> expr -> accept(this);
        // ACCEPT ARRAYS OF THE SAME TYPE AS THE IDENTIFIER
        if (decl -> type == parser::INT && current_expression_type == parser::INT_ARR) {
            current_scope->declare(decl->identifier, parser::INT_ARR, decl->line_number, 0);
        }
        else if (decl -> type == parser::REAL && current_expression_type == parser::REAL_ARR) {
            current_scope->declare(decl->identifier, parser::REAL_ARR, decl->line_number, 0);
        }
        else if (decl -> type == parser::BOOL && current_expression_type == parser::BOOL_ARR) {
            current_scope->declare(decl->identifier, parser::BOOL_ARR, decl->line_number, 0);
        }
        else if (decl -> type == parser::REAL && current_expression_type == parser::INT_ARR) {
            current_scope->declare(decl->identifier, parser::REAL_ARR, decl->line_number, 0);
        }
        else if (decl -> type == parser::STRING && current_expression_type == parser::STRING_ARR) {
            current_scope->declare(decl->identifier, parser::REAL_ARR, decl->line_number, 0);
        }
        // allow mismatched type in the case of declaration of int to real
        else if(decl -> type == parser::REAL && current_expression_type == parser::INT)
            current_scope->declare(decl->identifier, parser::REAL, decl->line_number, 0);

        // types match
        else if (decl -> type == current_expression_type)
            current_scope->declare(decl->identifier, decl->type, decl->line_number, 0);

        // types don't match
        else
            throw std::runtime_error("Found " + type_str(current_expression_type) + " on line " +
                                    std::to_string(decl->line_number) + " in definition of '" +
                                    decl -> identifier + "', expected " + type_str(decl->type) + ".");
    }
}

void SemanticAnalyser::visit(parser::ASTAssignmentNode *assign) {

    // Determine the inner-most scope in which the value is declared
    unsigned long j;
    for (j = scopes.size() - 1; !scopes[j] -> already_declared(assign->identifier); j--)
        if(j < 0)
            throw std::runtime_error("Identifier '" + assign->identifier + "' being reassigned on line " +
                                     std::to_string(assign->line_number) + " was never declared " +
                                     ((scopes.size() == 1) ? "globally." : "in this scope."));


    // Get the type of the originally declared variable
    parser::TYPE type = scopes[j]->type(assign->identifier);
    if (assign->is_array && !assign->require_input) {
        
        // Check if first position is available
        if (assign->first_position !=  nullptr) {
            assign->first_position->accept(this);
            // Check if current expr type is INT
            if (current_expression_type !=  parser::INT) {
                throw std::runtime_error("Invalid index type in array " +assign->identifier+
                    "on line "+std::to_string(assign->line_number)+". Expexted "+type_str(parser::INT)+ ", found "+ type_str(current_expression_type)+"."
                    );
            }
        }
        
        // Check if last position is available
        if (assign->last_position !=  nullptr) {
            assign->last_position->accept(this);
            // Check if current expr type is INT
            if (current_expression_type !=  parser::INT) {
                throw std::runtime_error("Invalid last index type in array " +assign->identifier+
                    "on line "+std::to_string(assign->line_number)+". Expexted "+type_str(parser::INT)+ ", found "+ type_str(current_expression_type)+"."
                    );
            }
            
        }
        
        switch(type) {
            case parser::INT_ARR:{
                long int i = 0;
                while (i < assign->array_size) {
                    // Visit the expression to update current type
                    assign->array_expr[i]->accept(this);

                    // allow mismatched type in the case of declaration of int to real
                    if (current_expression_type != parser::INT)
                        throw std::runtime_error("Mismatched type for '" + assign->identifier + "' on line " +
                                                std::to_string(assign->line_number) + ". Expected " + type_str(parser::INT) +
                                            ", found " + type_str(current_expression_type) + " in the elements.");
                    i++;
                }
                break;
            }
            case parser::REAL_ARR:{
                long int i = 0;
                while (i < assign->array_size) {
                    // Visit the expression to update current type
                    assign->array_expr[i]->accept(this);

                    // allow mismatched type in the case of declaration of int to real
                    if (type == parser::REAL_ARR && current_expression_type == parser::INT) {}

                    // otherwise throw error
                    else if (current_expression_type != parser::REAL)
                        throw std::runtime_error("Mismatched type for '" + assign->identifier + "' on line " +
                                                std::to_string(assign->line_number) + ". Expected " + type_str(parser::REAL) +
                                            ", found " + type_str(current_expression_type) + " in the elements.");
                    i++;
                }
                break;
            }
            case parser::BOOL_ARR:{
                long int i = 0;
                while (i < assign->array_size) {
                    // Visit the expression to update current type
                    assign->array_expr[i]->accept(this);

                    if (current_expression_type != parser::BOOL)
                        throw std::runtime_error("Mismatched type for '" + assign->identifier + "' on line " +
                                                std::to_string(assign->line_number) + ". Expected " + type_str(parser::BOOL) +
                                            ", found " + type_str(current_expression_type) + " in the elements.");
                    i++;
                }
                break;
            }
            case parser::STRING_ARR:{
                long int i = 0;
                while (assign->array_size > i) {
                    // Visit the expression to update current type
                    assign->array_expr[i]->accept(this);

                   if (current_expression_type != parser::STRING)
                        throw std::runtime_error("Mismatched type for '" + assign->identifier + "' on line " +
                                                std::to_string(assign->line_number) + ". Expected " + type_str(parser::STRING) +
                                            ", found " + type_str(current_expression_type) + " in the elements.");
                    i++;
                }
                break;
            }
            case parser::INT:
                break;
            case parser::REAL:
                break;
            case parser::BOOL:
                break;
            case parser::STRING:
                break;
        }
    }
    else if (!assign->is_array){
        if (assign->require_input) {
            // Visit the expression to get input statement
            assign->expr->accept(this);
        }
        else{
            // Visit the expression to update current type
            assign->expr->accept(this);

            // allow mismatched type in the case of declaration of int to real
            if (type == parser::REAL && current_expression_type == parser::INT) {}

            // otherwise throw error
            else if (current_expression_type != type)
                throw std::runtime_error("Mismatched type for '" + assign->identifier + "' on line " +
                                        std::to_string(assign->line_number) + ". Expected " + type_str(type) +
                                        ", found " + type_str(current_expression_type) + ".");
            }
        }
        else
            throw std::runtime_error("Mismatched type for '" + assign->identifier + "' on line " +
                                        std::to_string(assign->line_number) + ". Input statements support non array types but found " + type_str(type) + ".");
}


void SemanticAnalyser::visit(parser::ASTIncludeNode *includ){
	// Update current expression
    includ -> file_name;
    visitor::file_include(includ -> file_name);
}
void SemanticAnalyser::visit(parser::ASTPrintNode *print) {

    // Update current expression
    print -> expr -> accept(this);
}

void SemanticAnalyser::visit(parser::ASTReturnNode *ret) {

    // Update current expression
    ret -> expr -> accept(this);

    // If we are not global, check that we return current function return type
    if(!functions.empty() && current_expression_type != functions.top())
        throw std::runtime_error("Invalid return type on line " + std::to_string(ret->line_number) +
                                 ". Expected " + type_str(functions.top()) + ", found " +
                                 type_str(current_expression_type) + ".");
}

void SemanticAnalyser::visit(parser::ASTAppendNode* append)
{
    // Determine the inner-most scope in which the value is declared
    unsigned long j;
    for (j = scopes.size() - 1; !scopes[j] -> already_declared(append->identifier); j--)
        if(j < 0)
            throw std::runtime_error("Array '" + append->identifier + "' being appended on line " +
                                     std::to_string(append->line_number) + " was never declared " +
                                     ((scopes.size() == 1) ? "globally." : "in this scope."));


    // Get the type of the originally declared array
    parser::TYPE type = scopes[j]->type(append->identifier);
    
    // update current expr
    append->expression->accept(this);
    
    switch (type) {
        case parser::INT_ARR:
        {
            if (current_expression_type != parser::INT)
                throw std::runtime_error("Mismatched type for '" + append->identifier + "' on line " +
                                                std::to_string(append->line_number) + ". Expected to append " + type_str(parser::INT) +
                                            ", found expression of type " + type_str(current_expression_type) + ".");
            break;
        }
        case parser::REAL_ARR:
        {
            if (current_expression_type != parser::INT && current_expression_type != parser::REAL)
                throw std::runtime_error("Mismatched type for '" + append->identifier + "' on line " +
                                                std::to_string(append->line_number) + ". Expected to append " + type_str(parser::REAL) +
                                            ", found expression of type " + type_str(current_expression_type) + ".");
            break;
        }
        case parser::BOOL_ARR:
        {
            if (current_expression_type != parser::BOOL)
                throw std::runtime_error("Mismatched type for '" + append->identifier + "' on line " +
                                                std::to_string(append->line_number) + ". Expected to append " + type_str(parser::BOOL) +
                                            ", found expression of type " + type_str(current_expression_type) + ".");
            break;
        }
        case parser::STRING_ARR:
        {
            if (current_expression_type != parser::STRING)
                throw std::runtime_error("Mismatched type for '" + append->identifier + "' on line " +
                                                std::to_string(append->line_number) + ". Expected to append " + type_str(parser::STRING) +
                                            ", found expression of type " + type_str(current_expression_type) + ".");
            break;
        }
        default:
            throw std::runtime_error("Mismatched type for '" + append->identifier + "' on line " +
                                                std::to_string(append->line_number) + ". Expected to append to arrays only.");
    }
    
}


void SemanticAnalyser::visit(parser::ASTBlockNode *block) {

    // Create new scope
    scopes.push_back(new SemanticScope());

    // Check whether this is a function block by seeing if we have any current function
    // parameters. If we do, then add them to the current scope.
    for(auto param : current_function_parameters)
        scopes.back() -> declare(param.first, param.second, block->line_number, 0);

    // Clear the global function parameters vector
    current_function_parameters.clear();

    // Visit each statement in the block
    for(auto &stmt : block -> statements)
        stmt -> accept(this);

    // Close scope
    scopes.pop_back();
}

void SemanticAnalyser::visit(parser::ASTIfNode *ifnode) {

    // Set current type to if expression
    ifnode -> condition -> accept(this);

    // Make sure it is boolean
    if(current_expression_type != parser::BOOL)
        throw std::runtime_error("Invalid if-condition on line " + std::to_string(ifnode -> line_number)
                                 + ", expected boolean expression.");

    // Check the if block
    ifnode -> if_block -> accept(this);
    
    /**
     * @else_if statements
     */
    if(ifnode->else_if_block){
        int i = 0;
        while(ifnode->else_if_block[i]){
            ifnode->else_if_conditions[i] -> accept(this);
            // Make sure it is boolean
            if(current_expression_type != parser::BOOL)
                throw std::runtime_error("Invalid else if-condition on line " + std::to_string(ifnode -> line_number)
                                        + ", expected boolean expression.");
            // Check the if block
            ifnode -> else_if_block[i] ->accept(this);
            ++i;
        }
    }
    // If there is an else block, check it too
    if(ifnode -> else_block){
        ifnode -> else_block -> accept(this);
    }

}

void SemanticAnalyser::visit(parser::ASTWhileNode *whilenode) {

    // Set current type to while expression
    whilenode -> condition -> accept(this);

    // Make sure it is boolean
    if(current_expression_type != parser::BOOL)
        throw std::runtime_error("Invalid while-condition on line " + std::to_string(whilenode -> line_number)
                                 + ", expected boolean expression.");

    // Check the while block
    whilenode -> block -> accept(this);
}

void SemanticAnalyser::visit(parser::ASTFunctionDefinitionNode *func) {

    // First check that all enclosing scopes have not already defined the function
    for(auto &scope : scopes)
        if(scope->already_declared(func->identifier, func->signature)) {

            // Determine line number of error and the corresponding function signature
            int line = scope -> declaration_line(func -> identifier, func -> signature);
            std::string signature = "(";
            bool has_params = false;
            for(auto param : func -> signature) {
                has_params = true;
                signature += type_str(param) + ", ";
            }
            signature.pop_back();   // remove last whitespace
            signature.pop_back();   // remove last comma
            signature += ")";


            throw std::runtime_error("Error on line " + std::to_string(func->line_number) +
                                     ". Function " + func->identifier + signature +
                                     " already defined on line "+ std::to_string(line) + ".");
        }

    // Add function to symbol table
    scopes.back() -> declare(func->identifier, func->type, func->signature, func->line_number);

    // Push current function type onto function stack
    functions.push(func->type);

    // Empty and update current function parameters vector
    current_function_parameters.clear();
    current_function_parameters = func->parameters;

    // Check semantics of function block by visiting nodes
    func -> block -> accept(this);

    // Check that the function body returns
    if(!returns(func -> block))
        throw std::runtime_error("Function " + func->identifier + " defined on line " +
                                 std::to_string(func->line_number) + " is not guaranteed to "+
                                 "return a value.");

    // End the current function
    functions.pop();
}

void SemanticAnalyser::visit(parser::ASTLiteralNode<long int>*) {
    current_expression_type = parser::INT;
}

void SemanticAnalyser::visit(parser::ASTLiteralNode<long double>*) {
    current_expression_type = parser::REAL;
}

void SemanticAnalyser::visit(parser::ASTLiteralNode<bool>*) {
    current_expression_type = parser::BOOL;
}

void SemanticAnalyser::visit(parser::ASTLiteralNode<std::string>*) {
    current_expression_type = parser::STRING;
}


void SemanticAnalyser::visit(parser::ASTLiteralNode<long int*>*) {
    current_expression_type = parser::INT_ARR;
}

void SemanticAnalyser::visit(parser::ASTLiteralNode<long double*>*) {
    current_expression_type = parser::REAL_ARR;
}

void SemanticAnalyser::visit(parser::ASTLiteralNode<bool*>*) {
    current_expression_type = parser::BOOL_ARR;
}

void SemanticAnalyser::visit(parser::ASTLiteralNode<std::string*>*) {
    current_expression_type = parser::STRING_ARR;
}

void SemanticAnalyser::visit(parser::ASTBinaryExprNode* bin) {

    // Operator
    std::string op = bin -> op;
    int l = op.length(); // storing the length of the string
 
    int c
        = count(op.begin(), op.end(),
                ' '); // counting the number of whitespaces
 
    remove(op.begin(), op.end(),
           ' '); // removing all the whitespaces
 
    op.resize(l - c); // resizing the string to l-c
    // Visit left node first
    bin -> op = op;
    bin -> left -> accept(this);
    parser::TYPE l_type = current_expression_type;

    // Then right node
    bin -> right -> accept(this);
    parser::TYPE r_type = current_expression_type;

    // These only work for int/real
    if(op == "*" || op == "/" || op == "-" || op=="%" || op=="^" || op=="!#"){
        if((l_type != parser::INT && l_type != parser::REAL) ||
           (r_type != parser::INT && r_type != parser::REAL))
            throw std::runtime_error("Expected numerical operands for '" + op +
                                     "' operator on line " + std::to_string(bin->line_number) + ".");

        // If both int, then expression is int, otherwise real
        current_expression_type = (l_type == parser::INT && r_type == parser::INT) ?
                                  parser::INT : parser::REAL;
    }

    // + works for all types except bool
    else if(op == "+") {
        
        // If both string, no error
        if(l_type == parser::STRING || r_type == parser::STRING)
            current_expression_type = parser::STRING;
            
        else if(l_type == parser::BOOL || r_type == parser::BOOL)
            throw std::runtime_error("Invalid operand for '+' operator, expected numerical or string"
                                     " operand on line " + std::to_string(bin->line_number) + ".");

        // only one is string, error
        // allow string and other type addtion
        /*else if(l_type == parser::STRING || r_type == parser::STRING) 
            throw std::runtime_error("Mismatched operands for '+' operator, found " + type_str(l_type) +
                                     " on the left, but " + type_str(r_type) + " on the right (line " +
                                      std::to_string(bin->line_number) + ").");*/

        // real/int possibilities remain. If both int, then result is int, otherwise result is real
        else
            current_expression_type = (l_type == parser::INT && r_type == parser::INT) ?
                                      parser::INT : parser::REAL;
    }

    // and/or only work for bool
    else if(op == "and" || op == "or" || op == "&" || op == "&&" || op == "|" || op == "||") {
        if (l_type == parser::BOOL && r_type == parser::BOOL)
            current_expression_type = parser::BOOL;
        else throw std::runtime_error("Expected two boolean-type operands for '" + op + "' operator " +
                                      "on line " + std::to_string(bin->line_number) + ".");
    }

    // rel-ops only work for numeric types
    else if(op == "<" || op == ">" || op == "<=" || op == ">=" ) {
        if ((l_type != parser::REAL && l_type != parser::INT) ||
            (r_type != parser::REAL && r_type != parser::INT))
            throw std::runtime_error("Expected two numerical operands for '" + op + "' operator " +
                                     "on line " + std::to_string(bin->line_number) + ".");
        current_expression_type = parser::BOOL;
    }

    // == and != only work for like types
    else if(op == "==" || op == "!=") {
        if (l_type != r_type && (l_type != parser::REAL || r_type != parser::INT) &&
            (l_type != parser::INT || r_type != parser::REAL))
            throw std::runtime_error("Expected arguments of the same type '" + op + "' operator " +
                                     "on line " + std::to_string(bin->line_number) + ".");
        current_expression_type = parser::BOOL;
    }

    else
        throw std::runtime_error("Unhandled semantic error in binary operator '"+op+"' on line "+
								std::to_string(bin->line_number)+".");
}

void SemanticAnalyser::visit(parser::ASTIdentifierNode* id) {

    // Determine the inner-most scope in which the value is declared
    unsigned long i;
    for (i = scopes.size() - 1; !scopes[i] -> already_declared(id->identifier); i--)
        if(i <= 0)
            throw std::runtime_error("Identifier '" + id->identifier + "' appearing on line " +
                                     std::to_string(id->line_number) + " was never declared " +
                                     ((scopes.size() == 1) ? "globally." : "in this scope."));

    if (id ->array_position !=  nullptr) {
        // Eat expression
        id->array_position->accept(this);
        // Check index type
        if (current_expression_type != parser::INT) {
            throw std::runtime_error("Expected index of type" + type_str(parser::INT)+",  found "
                +type_str(current_expression_type)+" when indexing array " + id->identifier +" on line "+std::to_string(id->line_number)+".");
        }
        // If last position is given in index
        if (id->last_array_position !=  nullptr) {
            id->last_array_position->accept(this);
            if (current_expression_type != parser::INT) {
                throw std::runtime_error("Expected last index of type" + type_str(parser::INT)+",  found "
                    +type_str(current_expression_type)+" when indexing array " + id->identifier +" on line "+std::to_string(id->line_number)+".");
            }
            // Update current expression type
            current_expression_type = scopes[i]->type(id->identifier);
        }
        else
        // Assign new elemnt current_expression_type
        switch (scopes[i]->type(id->identifier)) {
            case parser::INT_ARR:
                current_expression_type = parser::INT;
                break;
            case parser::REAL_ARR:
                current_expression_type = parser::REAL;
                break;
            case parser::BOOL_ARR:
                current_expression_type = parser::BOOL;
                break;
            case parser::STRING_ARR:
                current_expression_type = parser::STRING;
                break;
            default:
                break;
        }
    }
    else{
        // Update current expression type
        current_expression_type = scopes[i]->type(id->identifier);
    }
}

void SemanticAnalyser::visit(parser::ASTUnaryExprNode* un) {

    // Determine expression type
    un -> expr -> accept(this);
    int l = un->unary_op.length(); // storing the length of the string
 
    int c
        = count(un->unary_op.begin(), un->unary_op.end(),
                ' '); // counting the number of whitespaces
 
    remove(un->unary_op.begin(), un->unary_op.end(),
           ' '); // removing all the whitespaces
 
    un->unary_op.resize(l - c); // resizing the string to l-c
    // Visit left node first
    // Handle different cases
    switch(current_expression_type){
        case parser::INT:
        case parser::REAL:
            if(un -> unary_op != "+" && un -> unary_op != "-")
                throw std::runtime_error("Operator '" + un -> unary_op + "' in front of numerical " +
                                         "expression on line " + std::to_string(un->line_number) + ".");
            break;
        case parser::BOOL:
            if(un -> unary_op != "not" && un -> unary_op != "!")
                throw std::runtime_error("Operator '" + un -> unary_op + "' in front of boolean " +
                                         "expression on line " + std::to_string(un->line_number) + ".");
            break;
        default:
            throw std::runtime_error("Incompatible unary operator '" + un -> unary_op + "' in front of " +
                                     "expression on line " + std::to_string(un->line_number) + ".");
    }
}

void SemanticAnalyser::visit(parser::ASTFunctionCallNode *func) {

    // Determine the signature of the function
    std::vector<parser::TYPE> signature;

    // For each parameter,
    for(auto param : func -> parameters) {

        // visit to update current expr type
        param -> accept(this);

        // add the type of current expr to signature
        signature.push_back(current_expression_type);
    }

    // Make sure the function exists in some scope i
    unsigned long i;
    for (i = scopes.size() - 1;
         !scopes[i] -> already_declared(func->identifier, signature);
         i--)
        if(i <= 0) {
            std::string func_name = func->identifier + "(";
            bool has_params = false;
            for(auto param : signature) {
                has_params = true;
                func_name += type_str(param) + ", ";
            }
            func_name.pop_back();   // remove last whitespace
            func_name.pop_back();   // remove last comma
            func_name += ")";
            throw std::runtime_error("Function '" + func_name + "' appearing on line " +
                                     std::to_string(func->line_number) + " was never declared " +
                                     ((scopes.size() == 1) ? "globally." : "in this scope."));
        }

    // Set current expression type to the return value of the function
    current_expression_type = scopes[i]->type(func->identifier, std::move(signature));
}


std::string type_str(parser::TYPE t) {

    switch(t){
        case parser::INT:
            return "int";
        case parser::REAL:
            return "float";
        case parser::BOOL:
            return "bool";
        case parser::STRING:
            return "string";
            case parser::INT_ARR:
            return "int[]";
        case parser::REAL_ARR:
            return "float[]";
        case parser::BOOL_ARR:
            return "bool[]";
        case parser::STRING_ARR:
            return "string[]";
        case parser::FILE_:
            return "file";
        default:
            throw std::runtime_error("Invalid type encountered.");
    }
}


// Determines whether a statement definitely returns or not
bool SemanticAnalyser::returns(parser::ASTStatementNode* stmt){

    // Base case: if the statement is a return statement, then it definitely returns
    if(dynamic_cast<parser::ASTReturnNode*>(stmt))
        return true;

    // For a block, if at least one statement returns, then the block returns
    if(auto block = dynamic_cast<parser::ASTBlockNode*>(stmt))
        for(auto &blk_stmt : block->statements)
            if(returns(blk_stmt))
                return true;

    // An if-(else) block returns only if both the if and the else statement return.
    if(auto ifstmt = dynamic_cast<parser::ASTIfNode*>(stmt))
        if(ifstmt -> else_block)
            return (returns(ifstmt->if_block) && returns(ifstmt->else_block));

    // A while block returns if its block returns
    if(auto whilestmt = dynamic_cast<parser::ASTWhileNode*>(stmt))
        return returns(whilestmt -> block);

    // Other statements do not return
    else return false;
}

void visitor::file_include(std::string fileargs){

    // Indefinite User input
    //for(;;){
    // Variables for user input
    std::string input_line, program;
    bool file_load = false;
    bool expr = false;

        // User prompt
        input_line = fileargs+".vn";

        // Remove leading/trailing whitespaces
        input_line = std::regex_replace(input_line, std::regex("^ +| +$"), "$1");

            //std::cout << input_line << std::endl; Unnecessary

            // If length <= 6, then the user specified no file
            if(input_line.size() == 0){
                std::cout << "File path expected as an argument" << std::endl;
            }

            else{

                // Get file directory
                std::string file_dir = input_line.substr(0);

                // Remove any whitespaces from that
                file_dir = std::regex_replace(file_dir, std::regex("^ +| +$"), "$1");

                // Read the file
                std::ifstream file;
                file.open(file_dir);

                if(!file)
                   throw std::runtime_error("Could not load file from \"" + file_dir + "\".");

                else {
                    // Convert whole program to std::string
                    std::string line;
                    while(std::getline(file, line))
                        program.append(line + "\n");

                    // Flag to indicate that this statement is for file loading
                    file_load = true;
                }

                file.close();
            }

        try {

            // Tokenise and initialise parser
            lexer::Lexer lexer(program);
            parser::Parser parser(&lexer);
            parser::ASTProgramNode *prog;

            // Try to parse as program
            try {
                prog = parser.parse_program();
            }

            // Catch by trying to parse as expression
            catch(const std::exception &e) {

                try {
                    // If expression ends with ';', get rid of it
                    if (program.back() == ';')
                        program.pop_back();

                    // Parse again, create program node manually
                    lexer::Lexer expr_lexer(program);
                    parser = parser::Parser(&expr_lexer, 0);  // do not consume first token
                    prog = new parser::ASTProgramNode(
                            std::vector<parser::ASTNode *>({parser.parse_expression()}));

                    expr = true;
                } catch(const std::exception &expr_e) {

                    // Throw original error
                    throw std::runtime_error(e.what());
                }
            }
    // Generate XML
            visitor::XMLVisitor xml_generator;
            xml_generator.visit(prog);


            // Try to analyse in a temporary copy of the global scope (just in case
            // the program is invalid)
            //visitor::SemanticScope temp = global::semantic_global_scope;
            //visitor::SemanticAnalyser temp_semantic_analyser(&temp);
            //temp_semantic_analyser.visit(prog);

            // If this succeeds, perform semantic analysis modifying global scope
            visitor::SemanticAnalyser semantic_analyser(&global::semantic_global_scope);
            semantic_analyser.visit(prog);
            
            // Interpreter
            visitor::Interpreter interpreter(&global::interpreter_global_scope);
            interpreter.visit(prog);

            // If loading file, show user that everything went well
            /*if (file_load)
                std::cout << "\nEnd of program Execution" << std::endl;*/ // Not necessary
        }
     // Catch exception and print error
        catch(const std::exception &e) {
            std::cerr << e.what() << std::endl;
        }
}
