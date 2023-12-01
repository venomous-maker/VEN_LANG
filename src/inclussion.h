#ifndef VENOM_INC_H
#define VENOM_INC_H
#include <iostream>
#include <fstream>
#include <regex>
#include <iomanip>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "visitor/xml_visitor.h"
#include "visitor/semantic_analysis.h"
#include "visitor/interpreter.h"
//#include "table/table.h"
    // Create Global Scopes
class global{
public:
    static visitor::SemanticScope semantic_global_scope;
    static visitor::InterpreterScope interpreter_global_scope;
    static std::string program;
    static std::string global_print_val;
	static std::string import_program;
    static parser::ASTProgramNode *prog;
    static parser::ASTExprNode *global_simple_expr;
    static bool generate_xml;
    static visitor::SemanticScope temp;
    static visitor::SemanticAnalyser temp_semantic_analyser;

    static visitor::SemanticAnalyser semantic_analyser;
    static visitor::Interpreter interpreter;
};
#endif
