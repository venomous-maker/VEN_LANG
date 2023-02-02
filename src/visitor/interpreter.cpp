//
// Created by Morgan OKumu.
//Updated by VENOM on 10/11/22 added more binary operators, fmod, power
//Update done to return float where n/m and m>n, n^m where m <0
//
/*#include <fstream>
#include <regex>
#include <iomanip>

//#include "../lexer/lexer.h"
#include "../parser/parser.h"
#include "../visitor/xml_visitor.h"
#include "../visitor/semantic_analysis.h"

//#include "../visitor/interpreter.h"
//#include "../table/table.h"*/
#include "../inclussion.h"
using namespace std;
#include <iostream>
#include "interpreter.h"
#include "math.h"
#include <cmath> //contains fmod e.t.c
//using namespace std;
using namespace visitor;

bool InterpreterScope::already_declared(std::string identifier) {
    return variable_symbol_table.find(identifier) != variable_symbol_table.end();
}

bool InterpreterScope::already_declared(std::string identifier, std::vector<parser::TYPE> signature) {

    auto funcs = function_symbol_table.equal_range(identifier);

    // If key is not present in multimap
    if(std::distance(funcs.first, funcs.second) == 0)
        return false;

    // Check signature for each function in multimap
    for (auto i = funcs.first; i != funcs.second; i++)
        if(std::get<0>(i->second) == signature)
            return true;

    // Function with matching signature not found
    return false;
}

void InterpreterScope::declare(std::string identifier, int int_value) {
    value_t value;
    value.i = int_value;
    variable_symbol_table[identifier] = std::make_pair(parser::INT, value);
}

void InterpreterScope::declare(std::string identifier, float real_value) {
    value_t value;
    value.r = real_value;
    variable_symbol_table[identifier] = std::make_pair(parser::REAL, value);
}

void InterpreterScope::declare(std::string identifier, bool bool_value) {
    value_t value;
    value.b = bool_value;
    variable_symbol_table[identifier] = std::make_pair(parser::BOOL, value);
}

void InterpreterScope::declare(std::string identifier, std::string string_value) {
    value_t value;
    value.s = string_value;
    variable_symbol_table[identifier] = std::make_pair(parser::STRING, value);
}

void InterpreterScope::declare(std::string identifier, std::vector<parser::TYPE> signature,
                               std::vector<std::string> variable_names, parser::ASTBlockNode* block) {

    function_symbol_table
            .insert(std::make_pair(identifier, std::make_tuple(signature,
                                                               variable_names,
                                                               block)));
}

parser::TYPE InterpreterScope::type_of(std::string identifier) {
    return variable_symbol_table[identifier].first;
}

value_t InterpreterScope::value_of(std::string identifier) {
    return variable_symbol_table[identifier].second;
}

std::vector<std::string> InterpreterScope::variable_names_of(std::string identifier,
                                                             std::vector<parser::TYPE> signature) {

    auto funcs = function_symbol_table.equal_range(identifier);
    auto i = funcs.first;
    // Match given signature to function in multimap
    for (i = funcs.first; i != funcs.second; i++)
        if(std::get<0>(i->second) == signature)
            return std::get<1>(i->second);
	return std::get<1>(i->second);
}

parser::ASTBlockNode* InterpreterScope::block_of(std::string identifier, std::vector<parser::TYPE> signature) {

    auto funcs = function_symbol_table.equal_range(identifier);

    // Match given signature to function in multimap
    for (auto i = funcs.first; i != funcs.second; i++)
        if(std::get<0>(i->second) == signature) {
            return std::get<2>(i->second);
        }

    return nullptr;
}

std::vector<std::tuple<std::string, std::string, std::string>> InterpreterScope::variable_list() {

    std::vector<std::tuple<std::string, std::string, std::string>> list;

    for(auto const &var : variable_symbol_table)
        switch(var.second.first){
            case parser::INT:
                list.emplace_back(std::make_tuple(
                        var.first, "int", std::to_string(var.second.second.i)));
                break;
            case parser::REAL:
                list.emplace_back(std::make_tuple(
                        var.first, "float", std::to_string(var.second.second.r)));
                break;
            case parser::BOOL:
                list.emplace_back(std::make_tuple(
                        var.first, "bool",  (var.second.second.b) ? "true" : "false"));
                break;
            case parser::STRING:
                list.emplace_back(std::make_tuple(
                        var.first, "string",  var.second.second.s));
                break;
        }

    return std::move(list);
}


Interpreter::Interpreter(){
    // Add global scope
    scopes.push_back(new InterpreterScope());
}

Interpreter::Interpreter(InterpreterScope* global_scope) {
    // Add global scope
    scopes.push_back(global_scope);
}

Interpreter::~Interpreter() = default;


void visitor::Interpreter::visit(parser::ASTProgramNode *prog) {

    // For each statement, accept
    for(auto &statement : prog -> statements)
        statement -> accept(this);
}

void visitor::Interpreter::visit(parser::ASTDeclarationNode *decl) {

    // Visit expression to update current value/type
    decl -> expr -> accept(this);

    // Declare variable, depending on type
    switch(decl -> type){
        case parser::INT:
            scopes.back()->declare(decl->identifier,
                                   current_expression_value.i);
            break;
        case parser::REAL:
            if(current_expression_type == parser::INT)
                scopes.back()->declare(decl->identifier,
                                       (float)current_expression_value.i);
            else
                scopes.back()->declare(decl->identifier,
                                        current_expression_value.r);
            break;
        case parser::BOOL:
            scopes.back()->declare(decl->identifier,
                                   current_expression_value.b);
            break;
        case parser::STRING:
            scopes.back()->declare(decl->identifier,
                                   current_expression_value.s);
            break;
    }
}

void visitor::Interpreter::visit(parser::ASTAssignmentNode *assign) {

    // Determine innermost scope in which variable is declared
    unsigned long i;
    for (i = scopes.size() - 1; !scopes[i] -> already_declared(assign->identifier); i--);

    // Visit expression node to update current value/type
    assign -> expr -> accept(this);

    // Redeclare variable, depending on type
    switch(scopes[i]->type_of(assign->identifier)){
        case parser::INT:
            scopes[i]->declare(assign->identifier,
                               current_expression_value.i);
            break;
        case parser::REAL:
            if(current_expression_type == parser::INT)
                scopes[i]->declare(assign->identifier,
                                   (float) current_expression_value.i);
            else
                scopes[i]->declare(assign->identifier,
                                   current_expression_value.r);
            break;
        case parser::BOOL:
            scopes[i]->declare(assign->identifier,
                               current_expression_value.b);
            break;
        case parser::STRING:
            scopes[i]->declare(assign->identifier,
                               current_expression_value.s);
            break;
    }
}

void visitor::Interpreter::visit(parser::ASTPrintNode *print){

    // Visit expression node to update current value/type
    print -> expr -> accept(this);

    // Print, depending on type
    switch(current_expression_type){
        case parser::INT:
            std::cout << current_expression_value.i;
            break;
        case parser::REAL:
            std::cout << current_expression_value.r;
            break;
        case parser::BOOL:
            std::cout << ((current_expression_value.b) ? "true" : "false");
            break;
        case parser::STRING:
            std::cout << current_expression_value.s;
            break;
    }
    std:cout<<"\n";
}
void file_include(std::string fileargs);
void visitor::Interpreter::visit(parser::ASTIncludeNode *includ){
	//includ->file_path/*->accept(this)*/;
	//cout<<current_expression_value.s;
	file_include(includ->file_name);
}
void visitor::Interpreter::visit(parser::ASTReturnNode *ret) {
    // Update current expression
    ret -> expr -> accept(this);
}

void visitor::Interpreter::visit(parser::ASTBlockNode *block) {

    // Create new scope
    scopes.push_back(new InterpreterScope());

    // Check whether this is a function block by seeing if we have any current function
    // parameters. If we do, then add them to the current scope.
    for(unsigned int i = 0; i < current_function_arguments.size(); i++){
        switch(current_function_arguments[i].first){
            case parser::INT:
                scopes.back() -> declare(current_function_parameters[i],
                                         current_function_arguments[i].second.i);
                break;
            case parser::REAL:
                scopes.back() -> declare(current_function_parameters[i],
                                         current_function_arguments[i].second.r);
                break;
            case parser::BOOL:
                scopes.back() -> declare(current_function_parameters[i],
                                         current_function_arguments[i].second.b);
                break;
            case parser::STRING:
                scopes.back() -> declare(current_function_parameters[i],
                                         current_function_arguments[i].second.s);
                break;
        }
    }

    // Clear the global function parameter/argument vectors
    current_function_parameters.clear();
    current_function_arguments.clear();

    // Visit each statement in the block
    for(auto &stmt : block -> statements)
        stmt -> accept(this);

    // Close scope
    scopes.pop_back();
}

void visitor::Interpreter::visit(parser::ASTIfNode *ifNode) {

    // Evaluate if condition
    ifNode -> condition -> accept(this);

    // Execute appropriate blocks
    if(current_expression_value.b)
        ifNode -> if_block -> accept(this);
    else{
        if(ifNode -> else_block)
            ifNode -> else_block -> accept(this);
    }

}

void visitor::Interpreter::visit(parser::ASTWhileNode *whileNode) {

    // Evaluate while condition
    whileNode -> condition -> accept(this);

    while(current_expression_value.b){
        // Execute block
        whileNode -> block -> accept(this);

        // Re-evaluate while condition
        whileNode -> condition -> accept(this);
    }
}

void visitor::Interpreter::visit(parser::ASTFunctionDefinitionNode *func) {

    // Add function to symbol table
    scopes.back() -> declare(func->identifier, func->signature,
                             func->variable_names, func->block);


}

void visitor::Interpreter::visit(parser::ASTLiteralNode<int> *lit) {
    value_t v;
    v.i = lit->val;
    current_expression_type = parser::INT;
    current_expression_value = std::move(v);
}

void visitor::Interpreter::visit(parser::ASTLiteralNode<float> *lit) {
    value_t v;
    v.r = lit->val;
    current_expression_type = parser::REAL;
    current_expression_value = std::move(v);
}

void visitor::Interpreter::visit(parser::ASTLiteralNode<bool> *lit) {
    value_t v;
    v.b = lit->val;
    current_expression_type = parser::BOOL;
    current_expression_value = std::move(v);
}

void visitor::Interpreter::visit(parser::ASTLiteralNode<std::string> *lit) {
    value_t v;
    v.s = lit->val;
    current_expression_type = parser::STRING;
    current_expression_value = std::move(v);
}

void visitor::Interpreter::visit(parser::ASTBinaryExprNode *bin) {

    // Operator
    std::string op = bin -> op;

    // Visit left node first
    bin -> left -> accept(this);
    parser::TYPE l_type = current_expression_type;
    value_t l_value = current_expression_value;

    // Then right node
    bin -> right -> accept(this);
    parser::TYPE r_type = current_expression_type;
    value_t r_value = current_expression_value;

    // Expression struct
    value_t v;

    // Arithmetic operators for now
    if(op == "+" || op == "-" || op == "*" || op == "/" || op == "%" || op == "^" || op =="!#") {
        // Two ints
        if(l_type == parser::INT && r_type == parser::INT){
            current_expression_type = parser::INT;
            if(op == "+")
                v.i = l_value.i + r_value.i;
            else if(op == "-")
                v.i = l_value.i - r_value.i;
            else if(op == "^")
            {
                if(r_value.i>=0){
		        v.i = round(pow(l_value.i, r_value.i));
		}else{
		        current_expression_type = parser::REAL; // Declare this as a float execution
		        v.r = 1/pow(l_value.i, abs(r_value.i)); //powers for values raised to -val
		}
            }
            else if(op == "*")
                v.i = l_value.i * r_value.i;
            else if (op == "%")
	        v.i = l_value.i % r_value.i;
	    else if (op == "!#"){ //factorial
	    	if(r_value.i>l_value.i){
	    		throw std::runtime_error("Factorial limit greater than the parent value encountered on line "
                                             + std::to_string(bin->line_number) + ".");
	    	}else{
		    	int mini_val = 1;
		    	float result = l_value.i;
		    	if (r_value.i==0) mini_val = 1;
		    	else mini_val = r_value.i;
		    	for(float dec_res=result-1;dec_res>=mini_val;dec_res--){
		    		result = result* dec_res;
		    	}
		    	current_expression_type = parser::REAL;
		    	v.r = result;
		    }
	    }
            else if(op == "/") {
                if(r_value.i == 0)
                    throw std::runtime_error("Division by zero encountered on line "
                                             + std::to_string(bin->line_number) + ".");
                if(l_value.i%r_value.i==0){
                      v.i = l_value.i / r_value.i;
                }
                else{//Declare as float if values are not divisible
                	current_expression_type = parser::REAL;
                	float l = l_value.i;
                	float r = r_value.i;
                	v.r = l / r;
                }
            }
        }
        // At least one real
        else if(l_type == parser::REAL || r_type == parser::REAL) {
            current_expression_type = parser::REAL;
            float l = l_value.r, r = r_value.r;
            if(l_type == parser::INT)
                l = l_value.i;
            if(r_type == parser::INT)
                r = r_value.i;
            if(op == "+")
                v.r = l+r;
            else if(op == "-")
                v.r = l-r;
            else if(op == "%")
                v.r = fmod(l,r);
            else if(op == "^")
                v.r = pow(l,r);
            else if(op == "*")
                v.r = l*r;
            else if(op == "!#"){
            if(r_value.r>l_value.r){
	    		throw std::runtime_error("Factorial limit greater than the parent value encountered on line "
                                             + std::to_string(bin->line_number) + ".");
	    	}else{
	    	float mini_val = 1.0;
	    	float result = l_value.r;
	    	if (r_value.i==0) mini_val = 1.0;
	    	else mini_val = r_value.r;
	    	for(int dec_res=result-1.0;dec_res>=mini_val;dec_res--){
	    		result = result* dec_res;
	    	}
	    	v.r = result;
	    }
            }
            else if(op == "/") {
                if(r == 0)
                    throw std::runtime_error("Division by zero encountered on line "
                                             + std::to_string(bin->line_number) + ".");
                v.r = l / r;
            }
        }
        // Remaining case is for strings
        else {
            current_expression_type = parser::STRING;
            v.s = l_value.s + r_value.s;
        }
    }
    // Now bool
    else if((op == "and" || op == "&&" || op == "&") || (op == "or" || op == "||" || op == "|")){
        current_expression_type = parser::BOOL;
        if(op == "and" || op == "&&" || op == "&")
            v.b = l_value.b && r_value.b;
        else if(op == "or" || op == "||" || op == "|")
            v.b = l_value.b || r_value.b;
    }

    // Now Comparator Operators
    else {
        current_expression_type = parser::BOOL;
        if(l_type == parser::BOOL)
                v.b = (op == "==") ? l_value.b == r_value.b : l_value.b != r_value.b;

        else if (l_type == parser::STRING)
                v.b = (op == "==") ? l_value.s == r_value.s : l_value.s != r_value.s;

        else{
            float l = l_value.r, r = r_value.r;
            if(l_type == parser::INT)
                l = l_value.i;
            if(r_type == parser::INT)
                r = r_value.i;
            if(op == "==")
                v.b = l == r;
            else if(op == "!=")
                v.b = l != r;
            else if(op == "<")
                v.b = l < r;
            else if(op == ">")
                v.b = l > r;
            else if(op == ">=")
                v.b = l >= r;
            else if(op == "<=")
                v.b = l <= r;
        }
    }


    // Update current expression
    current_expression_value = v;

}

void visitor::Interpreter::visit(parser::ASTIdentifierNode *id) {

    // Determine innermost scope in which variable is declared
    unsigned long i;
    for (i = scopes.size() - 1; !scopes[i] -> already_declared(id->identifier); i--);

    // Update current expression
    current_expression_type = scopes[i] -> type_of(id->identifier);
    current_expression_value = scopes[i] -> value_of(id->identifier);

}

void visitor::Interpreter::visit(parser::ASTUnaryExprNode *un) {

    // Update current expression
    un -> expr -> accept(this);

    switch(current_expression_type){
        case parser::INT:
            if(un->unary_op == "-")
            current_expression_value.i *= -1;
            break;
        case parser::REAL:
            if(un->unary_op == "-")
                current_expression_value.r *= -1;
            break;
        case parser::BOOL:
            current_expression_value.b ^= 1;
    }
}

void visitor::Interpreter::visit(parser::ASTFunctionCallNode *func) {

    // Determine the signature of the function
    std::vector<parser::TYPE> signature;
    std::vector<std::pair<parser::TYPE, value_t>> current_function_arguments;

    // For each parameter,
    for (auto param : func -> parameters) {

        // visit to update current expr type
        param->accept(this);

        // add the type of current expr to signature
        signature.push_back(current_expression_type);

        // add the current expr to the local vector of function arguments, to be
        // used in the creation of the function scope
        current_function_arguments.emplace_back(current_expression_type, current_expression_value);
    }

    // Update the global vector current_function_arguments
    for(auto arg : current_function_arguments)
        this -> current_function_arguments.push_back(arg);

    // Determine in which scope the function is declared
    unsigned long i;
    for (i = scopes.size() - 1;
         !scopes[i]->already_declared(func->identifier, signature);
         i--);

    // Populate the global vector of function parameter names, to be used in creation of
    // function scope
    current_function_parameters = scopes[i] -> variable_names_of(func->identifier, signature);

    // Visit the corresponding function block
    scopes[i] -> block_of(func->identifier, signature) -> accept(this);

}


std::pair<parser::TYPE, value_t> Interpreter::current_expr(){
    return std::move(std::make_pair(current_expression_type,
                                    current_expression_value));
};


std::string visitor::type_str(parser::TYPE t) {

    switch(t){
        case parser::INT:
            return "int";
        case parser::REAL:
            return "float";
        case parser::BOOL:
            return "bool";
        case parser::STRING:
            return "string";
        default:
            throw std::runtime_error("Invalid type encountered.");
    }
}

void file_include(std::string fileargs){

    // Indefinite User input
    //for(;;){
    // Variables for user input
    std::string input_line;
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
                    std::cout << "Could not load file from \"" + file_dir + "\"." << std::endl;

                else {
                    // Convert whole program to std::string
                    std::string line;
                    while(std::getline(file, line))
                        global::program.append(line + "\n");

                    // Flag to indicate that this statement is for file loading
                    file_load = true;
                }

                file.close();
            }

        try {

            // Tokenise and initialise parser
            lexer::Lexer lexer(global::program);
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
                    if (global::program.back() == ';')
                        global::program.pop_back();

                    // Parse again, create program node manually
                    lexer::Lexer expr_lexer(global::program);
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
            visitor::SemanticScope temp = global::semantic_global_scope;
            visitor::SemanticAnalyser temp_semantic_analyser(&temp);
            temp_semantic_analyser.visit(prog);

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
