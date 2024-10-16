/**
* @author Morgan OKumu.
* @update by VENOM on 10/11/22 added more binary operators, fmod, power
* @update done to return float where n/m and m>n, n^m where m <0
*/
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

void InterpreterScope::declare(std::string identifier, long int int_value) {
    value_t value;
    value.i = int_value;
    variable_symbol_table[identifier] = std::make_pair(parser::INT, value);
}

void InterpreterScope::declare(std::string identifier, long double real_value) {
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
// Arrays
void InterpreterScope::declare(std::string identifier, long int* int_value, unsigned long int size) {
    value_t value;
    value.i_ = int_value;
    variable_symbol_table[identifier] = std::make_pair(parser::INT_ARR, value);
    array_size_table[identifier] = std::make_pair(parser::INT_ARR, size);
}

void InterpreterScope::declare(std::string identifier, long double* real_value,  unsigned long int size) {
    value_t value;
    value.r_ = real_value;
    variable_symbol_table[identifier] = std::make_pair(parser::REAL_ARR, value);
    array_size_table[identifier] = std::make_pair(parser::REAL_ARR, size);
}

void InterpreterScope::declare(std::string identifier, bool* bool_value, unsigned long int size){
    value_t value;
    value.b_ = bool_value;
    variable_symbol_table[identifier] = std::make_pair(parser::BOOL_ARR, value);
    array_size_table[identifier] = std::make_pair(parser::BOOL_ARR, size);
}

void InterpreterScope::declare(std::string identifier, std::string* string_value,  unsigned long int size) {
    value_t value;
    value.s_ = string_value;
    variable_symbol_table[identifier] = std::make_pair(parser::STRING_ARR, value);
    array_size_table[identifier] = std::make_pair(parser::STRING_ARR, size);
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
        switch(var.second.first)
        {
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
            default:
                break;
        }
    return std::move(list);
}

std::vector<std::tuple<std::string, std::string, std::string*>> InterpreterScope::array_variable_list() {
    std::vector<std::tuple<std::string, std::string, std::string*>> list_array;

    for(auto const &var : variable_symbol_table)
        switch(var.second.first){
            case parser::INT_ARR:
            {
                int size = 0;
                std::string* result = nullptr;
                //std::string* result_temp = nullptr;
                result = new std::string[size + 1];
                while (var.second.second.i_[size]) {
                    result[size]   = std::to_string(var.second.second.i_[size]);
                    if (var.second.second.i_[size+1]) {
                        //result_temp = result;
                        ++size;
                        //std::string* temp = result;
                        result = (std::string*)realloc(result,  (size+1) * sizeof(std::string));   //new std::string[size+1];
                        //memcpy(result, temp, (size)*sizeof(std::string));
                        //delete[] temp;
                    }
                    else break;
                }
                
                list_array.emplace_back(std::make_tuple(
                        var.first, "int[]", result));
                delete[] result;
                break;
            }
            case parser::REAL_ARR:
            {
                int size = 0;
                std::string* result = nullptr;
                //std::string* result_temp = nullptr;
                result = new std::string[size + 1];
                while (var.second.second.r_[size]) {
                    result[size]   = std::to_string(var.second.second.r_[size]);
                    if (var.second.second.r_[size+1]) {
                        //result_temp = result;
                        ++size;
                        //std::string* temp = result;
                        //result = new std::string[size+1];
                        result = (std::string*)realloc(result,  (size+1) * sizeof(std::string));
                        //memcpy(result, temp, (size)*sizeof(std::string));
                        //delete[] temp;
                    }
                    else break;
                }
                
                list_array.emplace_back(std::make_tuple(
                        var.first, "float[]", result));
                delete[] result;
                break;
            }
            case parser::BOOL_ARR:
            {
                int size = 0;
                std::string* result = nullptr;
                //std::string* result_temp = nullptr;
                result = new std::string[size + 1];
                while (var.second.second.b_[size]) {
                    result[size] = (var.second.second.b_[size]) ? "true" : "false";
                    if (var.second.second.b_[size+1]) {
                        //result_temp = result;
                        ++size;
                        //std::string* temp = result;
                        //result = new std::string[size+1];
                        result = (std::string*)realloc(result,  (size+1) * sizeof(std::string));
                        //memcpy(result, temp, (size)*sizeof(std::string));
                        //delete[] temp;
                    }
                    else break;
                }
                
                list_array.emplace_back(std::make_tuple(
                        var.first, "bool[]", result));
                delete[] result;
                break;
            }
            case parser::STRING_ARR:
                list_array.emplace_back(std::make_tuple(
                        var.first, "string[]",  var.second.second.s_));
                break;
            default:
                break;
        }
    return std::move(list_array);
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

    // Declare variable, depending on type
    if (decl->is_array) {
        // IF ARRAY
        switch(decl -> type){
            case parser::INT:{
                long int *values = nullptr;
                
                int size = 0;
                // check if the first value occurs in the pointer
                if (decl->array_size > 0)
                {
                    // allocate size for first array element
                    values = (long int *) calloc((decl->array_size), sizeof(long int));
                }
                while (size < decl->array_size ) {
                    decl->array_expr[size]->accept(this);;
                    values[size] = current_expression_value.i;
                    
                    ++ size;
                }
                // change type to array type
                decl -> type = parser::INT_ARR;
                scopes.back()->declare(decl->identifier,
                                    values, (decl->array_size > 0) ? decl->array_size:0);
                //if (values[0]) free(values);
                break;
            }
            case parser::REAL:
            {
                long double *values = nullptr;
                
                unsigned long int size = 0;
                // check if the first value occurs in the pointer
                if (decl->array_size > 0)
                {
                    // allocate size for first array element
                    values = (long double *) calloc((decl->array_size), sizeof(long double));
                }
                
                while (size < decl->array_size) {
                    decl->array_expr[size]->accept(this);;
                    if(current_expression_type == parser::INT)
                        values[size] = (long double)current_expression_value.i;
                    else
                        values[size] = current_expression_value.r;
                    ++size;
                }
                // change type to array type
                decl -> type = parser::REAL_ARR;
                scopes.back()->declare(decl->identifier,
                                        values, (decl->array_size > 0) ? decl->array_size:0);
                //if (values[0]) free(values);
                break;
            }
            case parser::BOOL:
               {
                bool *values = nullptr;
                
                int size = 0;
                // check if the first value occurs in the pointer
                if (decl->array_size > 0)
                {
                    // allocate size for first array element
                    values = (bool *) calloc((decl->array_size), sizeof(bool));
                }
                
                while (size < decl->array_size) {
                    decl->array_expr[size]->accept(this);;
                    values[size] = current_expression_value.b;
                    ++size;
                }
                // change type to array type
                decl -> type = parser::BOOL_ARR;
                scopes.back()->declare(decl->identifier,
                                    values,  (decl->array_size > 0) ? decl->array_size:0);
                //if (values) free(values);
                break;
            }
            case parser::STRING:
                {
                std::string *values = nullptr;
                
                int size = 0;
                // check if the first value occurs in the pointer
                if (decl->array_size > 0)
                {
                    // allocate size for first array element
                    values = (std::string *) calloc((decl->array_size), sizeof(std::string));
                }
                //std::cout << decl->array_size;
                while (size < decl->array_size) {
                    decl->array_expr[size]->accept(this);;
                    
                    if (current_expression_value.s.empty())  
                    values[size] = "";
                    else values[size] = current_expression_value.s;
                    ++size;
                }
                // change type to array type
                decl -> type = parser::STRING_ARR;
                scopes.back()->declare(decl->identifier,
                                    values, (decl->array_size>0) ? decl->array_size:0);
               // if (values) free(values);
                break;
            }
            default:
                break;
        }
    }
    else
        // IF NOT ARRAY
        {
        // Visit expression to update current value/type
        decl -> expr -> accept(this);
        switch(decl -> type){
            case parser::INT:
                //  Accept arrays too
                if (current_expression_type ==  parser::INT_ARR)
                scopes.back()->declare(decl->identifier,
                                    current_expression_value.i_,  current_array_size);
                // else
                else
                    scopes.back()->declare(decl->identifier,
                                        current_expression_value.i);
                break;
            case parser::REAL:
                // Accept arrays too
                if (current_expression_type ==  parser::INT_ARR)
                    scopes.back()->declare(decl->identifier,
                                        (long double*)current_expression_value.i_,  current_array_size);
                else if (current_expression_type ==  parser::REAL_ARR)
                    scopes.back()->declare(decl->identifier,
                                        current_expression_value.r_,  current_array_size);
                                    
                else if(current_expression_type == parser::INT)
                    scopes.back()->declare(decl->identifier,
                                        (long double)current_expression_value.i);
                else
                    scopes.back()->declare(decl->identifier,
                                            current_expression_value.r);
                break;
            case parser::BOOL:
                // Arrays
                if (current_expression_type ==  parser::BOOL_ARR)
                     scopes.back()->declare(decl->identifier,
                                        current_expression_value.b_,  current_array_size);
                else scopes.back()->declare(decl->identifier,
                                    current_expression_value.b);
                break;
            case parser::STRING:
                // Arrays
                if (current_expression_type ==  parser::REAL_ARR)
                     scopes.back()->declare(decl->identifier,
                                        current_expression_value.s_,  current_array_size);
                else scopes.back()->declare(decl->identifier,
                                    current_expression_value.s);
                break;
            default:
                break;
        }
    }
}

void visitor::Interpreter::visit(parser::ASTAssignmentNode *assign) {

    // Determine innermost scope in which variable is declared
    long unsigned int first_position = 0;
    long unsigned int last_position = 0;
    if (assign->first_position !=  nullptr) {
        assign->first_position->accept(this);
        first_position = current_expression_value.i;
    }
    if (assign->last_position !=  nullptr) {
        assign->last_position->accept(this);
        last_position = current_expression_value.i;
    }
    unsigned long i;
    for (i = scopes.size() - 1; !scopes[i] -> already_declared(assign->identifier); i--);
    
    if (!assign->is_array)
    {
        // Visit expression node to update current value/type
        assign -> expr -> accept(this);
        if (assign->require_input) {
            switch (current_expression_type) {
                case parser::INT:
                {
                    std::cout << current_expression_value.i;
                    break;
                }
                case parser::BOOL:
                {
                    std::cout << (current_expression_value.b  ? "true" : "false");
                    break;
                }
                case parser::STRING:
                {
                    std::cout << current_expression_value.s;
                    break;
                }
                default:
                    break;
            }
            
            std::string input;
            std::cin >> input;
            switch (scopes[i]->type_of(assign->identifier)) {
                case parser::INT:
                    current_expression_type = parser::INT;
                    current_expression_value.i = std::stol(input);
                    break;
                case parser::REAL:
                    current_expression_type = parser::REAL;
                    current_expression_value.r = std::stold(input);
                    break;
                case parser::BOOL:
                    current_expression_type = parser::BOOL;
                    current_expression_value.b = (input == "true");
                    break;
                case parser::STRING:
                    current_expression_type = parser::STRING;
                    current_expression_value.s = input;
                    break;
                default:
                    break;
            }
        }

        // Redeclare variable, depending on type
        switch(scopes[i]->type_of(assign->identifier)){
            case parser::INT:
                scopes[i]->declare(assign->identifier,
                                current_expression_value.i);
                break;
            case parser::REAL:
                if(current_expression_type == parser::INT)
                    scopes[i]->declare(assign->identifier,
                                    (long double) current_expression_value.i);
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
            case parser::INT_ARR:
            {
                scopes[i]->declare(assign->identifier,
                                current_expression_value.i_, current_array_size);
                break;
            }
            case parser::REAL_ARR:
            {
                scopes[i]->declare(assign->identifier,
                                current_expression_value.r_, current_array_size);
                break;
            }
            case parser::BOOL_ARR:
            {
                scopes[i]->declare(assign->identifier,
                                current_expression_value.b_, current_array_size);
                break;
            }
            case parser::STRING_ARR:
                {
                scopes[i]->declare(assign->identifier,
                                current_expression_value.s_, current_array_size);
                break;
            }
        }
    }
    else{
        
        // Redeclare variable, depending on type
        switch(scopes[i]->type_of(assign->identifier)){
            case parser::INT:
                break;
            case parser::REAL:
                break;
            case parser::BOOL:
                break;
            case parser::STRING:
                break;
            case parser::INT_ARR:{
                // Get array values
                value_t value = scopes[i]->value_of(assign->identifier);
                // Deduct - from size to get the whole array size
                unsigned long int size = (scopes[i]->array_size_table[assign->identifier].second == 0) ? 0 : scopes[i]->array_size_table[assign->identifier].second;
                //  Check indices
                if (last_position != 0 && first_position > last_position)
                    throw std::runtime_error("Invalid array indexing in "+assign->identifier+"[ "+std::to_string(first_position)+": "
                      +std::to_string(last_position)+", on line "+std::to_string(assign->line_number)+".");
                if ((last_position >= size || first_position >= size) && !assign->change_range)
                    throw std::runtime_error("Array index out of bound in array "+assign->identifier+ " on line "+std::to_string(assign->line_number)
                      + ". Expected maximum size " +((size == 0)  ? "NULL":std::to_string(size-1))+", found "+std::to_string((last_position >= size) ? last_position
                        : first_position)+".");
                // Check if only one dimension is given
                if (assign->last_position == nullptr) {
                    // Check if we are changing a range or not
                    if (!assign->change_range) {
                        //  Check if expression exists
                        if (assign->array_expr[0]) {
                            //  Run the first expression only
                            assign -> array_expr[0] -> accept(this);
                            //  Check if the position exists then update
                            if (value.i_[first_position]) 
                                value.i_[first_position] = current_expression_value.i;
                        }
                    }
                    else{
                        long int *values = nullptr;
                        // Destroy size
                        size = 0;
                        // check if the first value occurs in the pointer
                        if (assign->array_size > 0)
                        {
                            // allocate size for first array element
                            values = (long int *) calloc(assign->array_size, sizeof(long int));
                        }
                        
                        while (size < assign->array_size) {
                            assign->array_expr[size]->accept(this);;
                            values[size] = current_expression_value.i;
                            size++;
                        }
                        value.i_ = values;
                        //if (values[0]) free(values);
                    }
                }
                else{
                    // If a range of values are given
                    int first = first_position;
                    int last = last_position;
                    // Allocate memory for section values
                    long int* section_values = (long int *) calloc((assign->array_size),  sizeof(long int));
                    int iter = 0;
                    // Check next element existance while looping
                    while (iter <= assign->array_size) {
                        //  Eat expression
                        assign->array_expr[iter]->accept(this);
                        section_values[iter] = current_expression_value.i;
                        ++iter;
                    }
                    // Reinitialize iter to fit the next iteration of swapping
                    iter = 0;
                    while (first < last) {
                        value.i_[first] = section_values[iter];
                        first++;
                        iter++;
                    }
                    // Free held section values
                    //free(section_values);
                }
                    scopes[i]->declare(assign->identifier,
                                value.i_, (value.i_)  ? size: 0);
                break;
            }
            case parser::REAL_ARR:
                {
                // Get array values
                value_t value = scopes[i]->value_of(assign->identifier);
                // Deduct - from size to get the whole array size
                unsigned long int size = (scopes[i]->array_size_table[assign->identifier].second == 0) ? 0 : 
                                                    scopes[i]->array_size_table[assign->identifier].second;
                // Check indinces
                if (last_position != 0 && first_position > last_position)
                    throw std::runtime_error("Invalid array indexing in "+assign->identifier+"[ "+std::to_string(first_position)+": "
                      +std::to_string(last_position)+", on line "+std::to_string(assign->line_number)+".");
                if ((last_position >= size || first_position >= size) && !assign->change_range)
                    throw std::runtime_error("Array index out of bound in array "+assign->identifier+ " on line "+std::to_string(assign->line_number)
                      + ". Expected maximum index " +((size == 0)  ? "NULL":std::to_string(size-1))+", found "+std::to_string((last_position >= size) ? last_position
                        : first_position)+".");
                // Check if only one dimension is given
                if (assign->last_position == 0) {
                    // Check if we are changing a range or not
                    if (!assign->change_range) {
                        //  Check if expression exists
                        if (assign->array_expr[0]) {
                            //  Run the first expression only
                            assign -> array_expr[0] -> accept(this);
                            //  Check if the position exists then update
                            if (value.r_[first_position]) {
                                if(current_expression_type == parser::INT)
                                    value.r_[first_position] = (long double)current_expression_value.i;
                                else
                                    value.r_[first_position] = current_expression_value.r;
                            }
                        }
                    }
                    else{
                        long double *values = nullptr;
                        
                        size = 0;
                        // check if the first value occurs in the pointer
                        if (assign->array_size > size)
                        {
                            // allocate size for first array element
                            values = (long double *) calloc(assign->array_size, sizeof(long double));
                        }
                        
                        while (assign->array_size > size) {
                            assign->array_expr[size]->accept(this);;
                            if(current_expression_type == parser::INT)
                                values[size] = (long double)current_expression_value.i;
                            else
                                values[size] = current_expression_value.r;
                            // check if the next value occurs in the pointer
                            size++;
                        }
                        value.r_ = values;
                        //if (values[0]) free(values);
                    }
                }
                else{
                    // If a range of values are given
                    int first = first_position;
                    int last = last_position;
                    // Allocate memory for section values
                    long double* section_values = (long double *) calloc((assign->array_size), sizeof(long double));
                    int iter = 0;
                    // Check next element existance while looping
                    while (iter <=  (assign->array_size)) {
                        //  Eat expression
                        assign->array_expr[iter]->accept(this);
                        if (current_expression_type ==  parser::INT) 
                            section_values[iter] = (long double) current_expression_value.i;
                        else section_values[iter] = current_expression_value.r;
                        ++iter;
                    }
                    // Reinitialize iter to fit the next iteration of swapping
                    iter = 0;
                    while (first < last) {
                        value.r_[first] = section_values[iter];
                        ++first;
                        ++iter;
                    }
                    // Free held section values
                    //free(section_values);
                }
                    scopes[i]->declare(assign->identifier,
                                value.r_, (value.r_)  ? size: 0);
                break;
            }
            case parser::BOOL_ARR:
                {
                // Get array values
                value_t value = scopes[i]->value_of(assign->identifier);
                // Check if only one dimension is given
                // Deduct - from size to get the whole array size
                unsigned long int size = (scopes[i]->array_size_table[assign->identifier].second == 0) ? 0 : 
                                                    scopes[i]->array_size_table[assign->identifier].second;
                // Check indices
                if (last_position != 0 && first_position > last_position)
                    throw std::runtime_error("Invalid array indexing in "+assign->identifier+"[ "+std::to_string(first_position)+": "
                      +std::to_string(last_position)+", on line "+std::to_string(assign->line_number)+".");
                if ((last_position >= size || first_position >= size) && !assign->change_range)
                    throw std::runtime_error("Array index out of bound in array "+assign->identifier+ " on line "+std::to_string(assign->line_number)
                      + ". Expected maximum index " +((size == 0)  ? "NULL":std::to_string(size-1))+", found "+std::to_string((last_position >= size) ? last_position
                        : first_position)+".");
                
                if (assign->last_position == 0) {
                    // Check if we are changing a range or not
                    if (!assign->change_range) {
                        //  Check if expression exists
                        if (assign->array_expr[0]) {
                            //  Run the first expression only
                            assign -> array_expr[0] -> accept(this);
                            //  Check if the position exists then update
                            if (value.b_[first_position]) 
                                value.b_[first_position] = current_expression_value.b;
                        }
                    }
                    else{
                        bool *values = nullptr;
                        bool *temp_values = nullptr;
                        
                        size = 0;
                        // check if the first value occurs in the pointer
                        if (assign->array_size > size)
                        {
                            // allocate size for first array element
                            values = (bool *) calloc(assign->array_size,  sizeof(bool));
                        }
                        
                        while (assign->array_size > size) {
                            assign->array_expr[size]->accept(this);;
                            values[size] = current_expression_value.b;
                            // check if the next value occurs in the pointer
                            ++size;
                        }
                        value.b_ = values;
                        //if (values[0]) free(values);
                    }
                }
                else{
                    // If a range of values are given
                    int first = first_position;
                    int last = last_position;
                    // Allocate memory for section values
                    bool* section_values = (bool *) calloc((assign->array_size), sizeof(bool));
                    int iter = 0;
                    // Check next element existance while looping
                    while (iter <= (assign->array_size)) {
                        //  Eat expression
                        assign->array_expr[iter]->accept(this);
                        section_values[iter] = current_expression_value.b;
                        ++iter;
                    }
                    // Reinitialize iter to fit the next iteration of swapping
                    iter = 0;
                    while (first < last) {
                        value.b_[first] = section_values[iter];
                        ++first;
                        ++iter;
                    }
                    // Free held section values
                    free(section_values);
                }
                    scopes[i]->declare(assign->identifier,
                                value.b_, (value.b_) ? size:0);
                break;
            }
            case parser::STRING_ARR:
                {
                // Get array values
                value_t value = scopes[i]->value_of(assign->identifier);
                // Deduct - from size to get the whole array size
                unsigned long int size = (scopes[i]->array_size_table[assign->identifier].second == 0) ? 0 : 
                                                    scopes[i]->array_size_table[assign->identifier].second;
                //  Check indices
                if (last_position != 0 && first_position > last_position)
                    throw std::runtime_error("Invalid array indexing in "+assign->identifier+"[ "+std::to_string(first_position)+": "
                      +std::to_string(last_position)+", on line "+std::to_string(assign->line_number)+".");
                if ((last_position >= size || first_position >= size) && !assign->change_range)
                    throw std::runtime_error("Array index out of bound in array "+assign->identifier+ " on line "+std::to_string(assign->line_number)
                      + ". Expected maximum index " +((size == 0)  ? "NULL":std::to_string(size-1))+", found "+std::to_string((last_position >= size) ? last_position
                        : first_position)+".");
                // Check if only one dimension is given
                if (assign->last_position == 0) {
                    // Check if we are changing a range or not
                    if (!assign->change_range) {
                        //  Check if expression exists
                        if (assign->array_expr[0]) {
                            //  Run the first expression only
                            assign -> array_expr[0] -> accept(this);
                            //  Check if the position exists then update
                            // This position should be updated
                            if (value.s_) 
                                value.s_[first_position] = current_expression_value.s;
                        }
                    }
                    else{
                        std::string *values = nullptr;
                        
                        size = 0;
                        // check if the first value occurs in the pointer
                        if (assign->array_size > size)
                        {
                            // allocate size for first array element
                            values = (std::string *) calloc(assign->array_size, sizeof(std::string));
                        }
                        
                        while (assign->array_size > size) {
                            assign->array_expr[size]->accept(this);;
                            values[size] = current_expression_value.s;
                            ++size;
                        }
                        value.s_ = values;
                        //if (values) free(values);
                    }
                }
                else{
                    // If a range of values are given
                    int first = first_position;
                    int last = last_position;
                    // Allocate memory for section values
                    std::string* section_values = (std::string *) calloc( (assign->array_size), sizeof(std::string));
                    int iter = 0;
                    // Check next element existance while looping
                    while ( (assign->array_size) >= iter) {
                        //  Eat expression
                        assign->array_expr[iter]->accept(this);
                        section_values[iter] = current_expression_value.s;
                        ++iter;
                    }
                    // Reinitialize iter to fit the next iteration of swapping
                    iter = 0;
                    while (first < last) {
                        value.s_[first] = section_values[iter];
                        ++first;
                        ++iter;
                    }
                    // Free held section values
                    // free(section_values);
                }
                    scopes[i]->declare(assign->identifier,
                                value.s_, (value.s_) ? size:0);
                break;
            }
        }
    }
}

void visitor::Interpreter::visit(parser::ASTAppendNode* append)
{
    unsigned long i;
    for (i = scopes.size() - 1; !scopes[i] -> already_declared(append->identifier); i--);
    if(i < 0)
        throw std::runtime_error("Array '" + append->identifier + "' being appended on line " +
                                    std::to_string(append->line_number) + " was never declared " +
                                    ((scopes.size() == 1) ? "globally." : "in this scope."));
    
    append->expression->accept(this);
    switch(scopes[i]->type_of(append->identifier)){
        case parser::INT_ARR:
        {
            if (current_expression_type != parser::INT)
                throw std::runtime_error("Mismatched type for '" + append->identifier + "' on line " +
                                                std::to_string(append->line_number) + ". Expected to append " + type_str(parser::INT) +
                                            ", found expression of type " + type_str(current_expression_type) + ".");
            auto size = scopes[i]->array_size_table[append->identifier].second;
            auto value = scopes[i]->value_of(append->identifier);
            value.i_ = (long int*) realloc(value.i_,  sizeof(long int) * (size+1));
            value.i_[size] = current_expression_value.i;
            scopes[i]->declare(append->identifier,
                                value.i_, (value.i_) ? size+1:0);
            break;
        }
        case parser::REAL_ARR:
        {
            if (current_expression_type != parser::INT && current_expression_type != parser::REAL)
                throw std::runtime_error("Mismatched type for '" + append->identifier + "' on line " +
                                                std::to_string(append->line_number) + ". Expected to append " + type_str(parser::REAL) +
                                            ", found expression of type " + type_str(current_expression_type) + ".");
            auto size = scopes[i]->array_size_table[append->identifier].second;
            auto value = scopes[i]->value_of(append->identifier);
            value.r_ = (long double*) realloc(value.r_,  sizeof(long double) * (size+1));
            value.r_[size] = (current_expression_type == parser::INT) ? (long double)current_expression_value.i : current_expression_value.r;
            scopes[i]->declare(append->identifier,
                                value.r_, (value.r_) ? size+1:0);
            break;
        }
        case parser::BOOL_ARR:
        {
            if (current_expression_type != parser::BOOL)
                throw std::runtime_error("Mismatched type for '" + append->identifier + "' on line " +
                                                std::to_string(append->line_number) + ". Expected to append " + type_str(parser::BOOL) +
                                            ", found expression of type " + type_str(current_expression_type) + ".");
            auto size = scopes[i]->array_size_table[append->identifier].second;
            auto value = scopes[i]->value_of(append->identifier);
            value.b_ = (bool*) realloc(value.b_,  sizeof(bool) * (size+1));
            value.b_[size] = current_expression_value.b;
            scopes[i]->declare(append->identifier,
                                value.b_, (value.b_) ? size+1:0);
            break;
        }
        case parser::STRING_ARR:
        {
            if (current_expression_type != parser::STRING)
                throw std::runtime_error("Mismatched type for '" + append->identifier + "' on line " +
                                                std::to_string(append->line_number) + ". Expected to append " + type_str(parser::STRING) +
                                            ", found expression of type " + type_str(current_expression_type) + ".");
            auto size = scopes[i]->array_size_table[append->identifier].second;
            auto value = scopes[i]->value_of(append->identifier);
            if (value.s_ !=  nullptr) {
                value.s_ = (std::string*) realloc(value.s_,  sizeof(std::string) * (size+1));
            }
            else value.s_ = (std::string *) calloc((1), sizeof(std::string));
            value.s_[size] = current_expression_value.s;
            scopes[i]->declare(append->identifier,
                                value.s_, (value.s_) ? size+1:0);
            break;
        }
        default:
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
        case parser::INT_ARR:{
            int i = 0;
            // Output array element
           // long int * l = current_expression_value.i_;
            std::cout << "[";
            while (i < current_array_size) {
                std::cout << current_expression_value.i_[i];
                if (current_array_size > i+1) std::cout << ",";
                else break;
                ++i;
            }
            std::cout << "]";
            break;
        }
        case parser::REAL_ARR:{
            int i = 0;
            // Output array element
            std::cout << "[";
            while (i < current_array_size) {
                std::cout << current_expression_value.r_[i];
                if (current_array_size > i+1) std::cout << ","; else break;
                i++;
            }
            std::cout << "]";
            break;
        }
        case parser::BOOL_ARR:{
            int i = 0;
            // Output array element
            std::cout << "[";
            while (current_array_size > i) {
                std::cout << ((current_expression_value.b_[i]) ? "true" : "false");
                if (current_array_size > i+1) std::cout << ","; else break;
                i++;
            }
            std::cout << "]";
            break;
        }
        case parser::STRING_ARR:{
            int i = 0;
            // Output array element
            //int * temp = (int*)current_expression_value.s_;
            std::cout << "[";
            while (current_array_size > i) {
                std::cout << " "<< current_expression_value.s_[i];
                if (current_array_size > i+1) std::cout << ",";
                else break;
                i++;
            }
            std::cout << "]";
            break;
        }
    }
    if(global::global_print_val != "printf")
    std:cout<<"\n";
}

// void file_include(std::string fileargs);
void visitor::Interpreter::visit(parser::ASTIncludeNode *includ){
    /**
     *@Nothing here
     **/
     
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
            case parser::INT_ARR:
                scopes.back() -> declare(current_function_parameters[i],
                                         current_function_arguments[i].second.i_);
                break;
            case parser::REAL_ARR:
                scopes.back() -> declare(current_function_parameters[i],
                                         current_function_arguments[i].second.r_);
                break;
            case parser::BOOL_ARR:
                scopes.back() -> declare(current_function_parameters[i],
                                         current_function_arguments[i].second.b_);
                break;
            case parser::STRING_ARR:
                scopes.back() -> declare(current_function_parameters[i],
                                         current_function_arguments[i].second.s_);
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
    // Captures execution in else if to make else statement reachable
    bool in_else_if = false;
    bool if_ = false;
    // Execute appropriate blocks
    if(current_expression_value.b) {
        if_ = true;
        ifNode -> if_block -> accept(this);
    }
    // if if not executed
    if(ifNode -> else_if_block && !if_){
        // else if statements
        int i = 0;
        while(ifNode->else_if_block[i]){
            ifNode->else_if_conditions[i] -> accept(this);
            // Execute appropriate blocks
            if(current_expression_value.b){
                // Else if is reached
                in_else_if = true;
                ifNode -> else_if_block[i] ->accept(this);
                break;
            }
            ++i;
        }
    }
    // If else-if and if not executed proceed to else
    if(!in_else_if && !if_){
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

void visitor::Interpreter::visit(parser::ASTLiteralNode<long int> *lit) {
    value_t v;
    v.i = lit->val;
    current_expression_type = parser::INT;
    current_expression_value = std::move(v);
    current_array_size = 0;
}

void visitor::Interpreter::visit(parser::ASTLiteralNode<long double> *lit) {
    value_t v;
    v.r = lit->val;
    current_expression_type = parser::REAL;
    current_expression_value = std::move(v);
    current_array_size = 0;
}

void visitor::Interpreter::visit(parser::ASTLiteralNode<bool> *lit) {
    value_t v;
    v.b = lit->val;
    current_expression_type = parser::BOOL;
    current_expression_value = std::move(v);
    current_array_size = 0;
}

void visitor::Interpreter::visit(parser::ASTLiteralNode<std::string> *lit) {
    value_t v;
    v.s = lit->val;
    current_expression_type = parser::STRING;
    current_expression_value = std::move(v);
    current_array_size = 0;
}

void visitor::Interpreter::visit(parser::ASTLiteralNode<long int*> *lit) {
    value_t v;
    v.i_ = lit->val;
    current_expression_type = parser::INT_ARR;
    current_expression_value = std::move(v);
}

void visitor::Interpreter::visit(parser::ASTLiteralNode<long double*> *lit) {
    value_t v;
    v.r_ = lit->val;
    current_expression_type = parser::REAL_ARR;
    current_expression_value = std::move(v);
}

void visitor::Interpreter::visit(parser::ASTLiteralNode<bool*> *lit) {
    value_t v;
    v.b_ = lit->val;
    current_expression_type = parser::BOOL_ARR;
    current_expression_value = std::move(v);
}

void visitor::Interpreter::visit(parser::ASTLiteralNode<std::string*> *lit) {
    value_t v;
    v.s_ = lit->val;
    current_expression_type = parser::STRING_ARR;
    current_expression_value = std::move(v);
}

void visitor::Interpreter::visit(parser::ASTBinaryExprNode *bin) {
    //cout << bin -> op <<endl;
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
    if(op == "+" || op == "-" || op == "*" || op == "/" || op == "%" || op == "^" || op =="!#" || op == "//") {
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
                    long int mini_val = 1;
                    long double result = l_value.i;
                    if (r_value.i==0) mini_val = 1;
                    else mini_val = r_value.i;
                    for(long double dec_res=result-1;dec_res>=mini_val;dec_res--){
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
                	long double l = l_value.i;
                	long double r = r_value.i;
                	v.r = l / r;
                }
            }
        }
        // At least one real
        else if(l_type == parser::REAL || r_type == parser::REAL) {
            current_expression_type = parser::REAL;
            long double l = l_value.r, r = r_value.r;
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
	    	long double mini_val = 1.0;
	    	long double result = l_value.r;
	    	if (r_value.i==0) mini_val = 1.0;
	    	else mini_val = r_value.r;
	    	for(long int dec_res=result-1.0;dec_res>=mini_val;dec_res--){
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
            std::string l = l_value.s, r = r_value.s;
            if(l_type == parser::INT)
                l = std::to_string(l_value.i);
            if(r_type == parser::INT)
                r = std::to_string(r_value.i);
            if(l_type == parser::REAL)
                l = std::to_string(l_value.r);
            if(r_type == parser::REAL)
                r = std::to_string(r_value.r);
            if(l_type == parser::BOOL)
                l = l_value.b  ? "true" : "false";
            if(r_type == parser::BOOL)
                r = r_value.b ? "true" : "false";
            v.s = l + r;
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
        
        else if(l_type == parser::BOOL_ARR)
                v.b = (op == "==") ? l_value.b_ == r_value.b_ : l_value.b_ != r_value.b_;

        else if (l_type == parser::STRING_ARR)
                v.b = (op == "==") ? l_value.s_ == r_value.s_ : l_value.s_ != r_value.s_;
                
        else if(l_type == parser::REAL_ARR)
                v.b = (op == "==") ? l_value.r_ == r_value.r_ : l_value.r_ != r_value.r_;

        else if (l_type == parser::INT_ARR)
                v.b = (op == "==") ? l_value.i_ == r_value.i_ : l_value.i_ != r_value.i_;

        else{
           long double l = l_value.r, r = r_value.r;
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
            else if(op == ">" )
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
    // Check if array index is given
    if (id->array_position !=  nullptr) {
        // Eat Expression
        id->array_position->accept(this);
        long int index = current_expression_value.i;
        long int last_index = 0;
        if (id->last_array_position != nullptr) {
            // Get expression
            id->last_array_position->accept(this);
            last_index = current_expression_value.i;
        }
        // Get type of the identifier
        if (scopes[i]->array_size_table.count(id->identifier))
            current_array_size = scopes[i]->array_size_table[id->identifier].second;
        
        parser::TYPE type = scopes[i] -> type_of(id->identifier);
        switch (type) {
            case parser::INT_ARR:
            {
                value_t index_value;
                if (id->last_array_position == nullptr) {
                    current_expression_type = parser::INT;
                    if (current_array_size <= index)
                        throw std::runtime_error("Array index out of bound when indexing '" + id->identifier
                        + "'. Expected maximum index of "+((current_array_size == 0) ? "NULL":std::to_string( current_array_size-1))+", found an invalid index "
                        + std::to_string(index)+" on line "+ std::to_string(id->line_number)+".");
                    index_value.i = scopes[i] -> value_of(id->identifier).i_[index];
                }else{
                    current_expression_type = parser::INT_ARR;
                    if (current_array_size <= last_index)
                        throw std::runtime_error("Array index out of bound when indexing '" + id->identifier
                        + "'. Expected maximum index of "+((current_array_size == 0) ? "NULL":std::to_string( current_array_size-1))+", found an invalid index "
                        + std::to_string(last_index)+" on line "+ std::to_string(id->line_number)+".");
                    long int *i_ = (long int*) calloc(last_index-index, sizeof(long int));
                    long int iter = 0;
                    // Update array size
                    current_array_size = last_index - index;
                    for (   ;index < last_index; iter++  ) 
                        i_[iter] = scopes[i] -> value_of(id->identifier).i_[index++];
                    index_value.i_ = i_;
                }
                current_expression_value = index_value;
                break;
            }
            case parser::REAL_ARR:
            {
                value_t index_value;
                if (id->last_array_position == nullptr) {
                    current_expression_type = parser::REAL;
                    if (current_array_size <= index)
                        throw std::runtime_error("Array index out of bound when indexing " + id->identifier
                        + ".Expected maximum index of "+((current_array_size == 0) ? "NULL":std::to_string( current_array_size-1))+", found an invalid index "
                        + std::to_string(index)+" on line "+ std::to_string(id->line_number)+".");
                    index_value.r = scopes[i] -> value_of(id->identifier).r_[index];
                }else{
                    current_expression_type = parser::REAL_ARR;
                    if (current_array_size <= last_index)
                        throw std::runtime_error("Array index out of bound when indexing " + id->identifier
                        + ".Expected maximum index of "+((current_array_size == 0) ? "NULL":std::to_string( current_array_size-1))+", found an invalid index "
                        + std::to_string(last_index)+" on line "+ std::to_string(id->line_number)+".");
                    long double *i_ = (long double*) calloc(last_index-index, sizeof(long double));
                    long int iter = 0;
                    // Update array size
                    current_array_size = last_index - index;
                    
                    for (   ;index < last_index; iter++  ) 
                        i_[iter] = scopes[i] -> value_of(id->identifier).r_[index++];
                    index_value.r_ = i_;
                    
                }
                current_expression_value = index_value;
                break;
            }
            case parser::BOOL_ARR:
            {
                value_t index_value;
                if (id->last_array_position == nullptr) {
                    current_expression_type = parser::BOOL;
                    if (current_array_size <= index)
                        throw std::runtime_error("Array index out of bound when indexing " + id->identifier
                        + ".Expected maximum index of "+((current_array_size == 0) ? "NULL":std::to_string( current_array_size-1))+", found an invalid index "
                        + std::to_string(index)+" on line "+ std::to_string(id->line_number)+".");
                    index_value.b = scopes[i] -> value_of(id->identifier).b_[index];
                }else{
                    current_expression_type = parser::BOOL_ARR;
                    if (current_array_size <= last_index)
                        throw std::runtime_error("Array index out of bound when indexing " + id->identifier
                        + ".Expected maximum index of "+((current_array_size == 0) ? "NULL":std::to_string( current_array_size-1))+", found an invalid index "
                        + std::to_string(last_index)+" on line "+ std::to_string(id->line_number)+".");
                    bool *i_ = (bool*) calloc(last_index-index, sizeof(bool));
                    long int iter = 0;
                    // Update array size
                    current_array_size = last_index - index;
                    for (   ;index < last_index; iter++  ) 
                        i_[iter] = scopes[i] -> value_of(id->identifier).b_[index++];
                    index_value.b_ = i_;
                }
                current_expression_value = index_value;
                break;
            }
            case parser::STRING_ARR:
            {
                value_t index_value;
                if (id->last_array_position == nullptr) {
                    current_expression_type = parser::STRING;
                    if (current_array_size <= index)
                        throw std::runtime_error("Array index out of bound when indexing " + id->identifier
                        + ".Expected maximum index of "+((current_array_size == 0) ? "NULL":std::to_string( current_array_size-1))+", found an invalid index "
                        + std::to_string(index)+" on line "+ std::to_string(id->line_number)+".");
                    index_value.s = scopes[i] -> value_of(id->identifier).s_[index];
                }else{
                    current_expression_type = parser::STRING_ARR;
                    if (current_array_size <= last_index)
                        throw std::runtime_error("Array index out of bound when indexing " + id->identifier
                        + ".Expected maximum index of "+((current_array_size == 0) ? "NULL":std::to_string( current_array_size-1))+", found an invalid index "
                        + std::to_string(last_index)+" on line "+ std::to_string(id->line_number)+".");
                    std::string* i_ = (std::string*) calloc(last_index-index, sizeof(std::string));
                    long int iter = 0;
                    // Update array size
                    current_array_size = last_index - index;
                    for (   ;index < last_index; iter++  ) 
                        i_[iter] = scopes[i] -> value_of(id->identifier).s_[index++];
                    index_value.s_ = i_;
                }
                current_expression_value = index_value;
                break;
            }
            default:
                break;
        }
        
    }
    else{
        current_expression_type = scopes[i] -> type_of(id->identifier);
        current_expression_value = scopes[i] -> value_of(id->identifier);
        // Check if an array then get size
        if (scopes[i]->array_size_table.count(id->identifier))
            current_array_size = scopes[i]->array_size_table[id->identifier].second;
    }

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
        default:
            break;
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

unsigned long visitor::Interpreter::get_current_array_size()
{
    return this->current_array_size;
}


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

