#include <iostream>
#include <fstream>
#include <regex>
#include <iomanip>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "visitor/xml_visitor.h"
#include "visitor/semantic_analysis.h"
#include "visitor/interpreter.h"
#include "inclussion.h"
#include "table/table.h"
#include "parser/parser.h"
using namespace std;
/**
 * The main function implements the REPL interface.
 * The
 * @return 0
 */
/*
* An update by Morgan okumu on clear screen command, help_menu function, console_args function
*/
// Global vars
visitor::SemanticScope global::semantic_global_scope;
visitor::InterpreterScope global::interpreter_global_scope;
std::string global::program;
parser::ASTExprNode *global::global_simple_expr;
std::string global::global_print_val;
bool global::generate_xml = false;
visitor::SemanticScope global::temp = global::semantic_global_scope;
visitor::SemanticAnalyser global::temp_semantic_analyser(&temp);
visitor::SemanticAnalyser global::semantic_analyser(&global::semantic_global_scope);
visitor::Interpreter global::interpreter(&global::interpreter_global_scope);
// This function displays the help menu
std::string version = "1.0.0";// Official update version
string date = "30th January 2023"; //Official update release
void help_menu();
// This function accepts the console argument as a file name
void console_args(std::string fileargs);
int main(int args, char* argv[]) {
	if (args<=1){
		// REPL Greeting
		std::cout << "\e[1mV3N0M\e[0m " <<version<<" ["<<date<<"]" << std::endl;
		std::cout << "Type \"#help\" for more information." << std::endl;
	}

//visitor::SemanticScope semantic_global_scope;
//visitor::InterpreterScope interpreter_global_scope;

    // Indefinite User input
    if (args>1) {
        int args_ = 1;
        while (args_ < args) {
        std::string arg = argv[args_];
    	if(arg=="-h" || arg =="--h" || arg=="--help" || arg == "-help") {help_menu(); break;}
        if (arg == "-xml" || arg == "--xml") global::generate_xml = true;
    	if (args_+1 == args) {console_args(argv[1]);}
         args_++;
        }
    }
    else for(;;){

        // Variables for user input
        std::string input_line;
        bool file_load = false;
        bool expr = false;

        // User prompt
        std::cout << "\n>>> _\b";
        std::getline(std::cin, input_line);

        // Remove leading/trailing whitespaces
        input_line = std::regex_replace(input_line, std::regex("^ +| +$"), "$1");

        // Quit
        if(input_line == "#quit"){
            break;
        }

        // Help
        else if(input_line == "#help"){
		help_menu();
        }

        // Load File
        else if(input_line.substr(0, 5) == "#load"){
            std::cout << input_line << std::endl;

            // If length <= 6, then the user specified no file
            if(input_line.size() <= 6){
                std::cout << "File path expected after '#load'." << std::endl;
            }

            else{

                // Get file directory
                std::string file_dir = input_line.substr(6);

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
        }

        // Symbol Table
        else if(input_line == "#st"){
            std::cout << "\e[1mCurrently Declared Variables and Functions\e[0m" << std::endl;
            TextTable vt('-', '|', '+');
            vt.add("Name");
            vt.add("Type");
            vt.add("Current Value");
            vt.endOfRow();
            for(auto var : global::interpreter_global_scope.variable_list()) {
                vt.add(std::get<0>(var));
                vt.add(std::get<1>(var));
                vt.add(std::get<2>(var));
                vt.endOfRow();
            }
            for(auto var : global::interpreter_global_scope.array_variable_list()) {
                vt.add(std::get<0>(var));
                vt.add(std::get<1>(var));
                vt.add("[*]");
                vt.endOfRow();
            }
            vt.setAlignment(2, TextTable::Alignment::RIGHT);
            std::cout << vt << std::endl;

            TextTable ft('-', '|', '+');
            ft.add("Name/Signature");
            ft.add("Return Type");
            ft.endOfRow();

            for(auto func : global::semantic_global_scope.function_list()) {
                ft.add(func.first);
                ft.add(func.second);
                ft.endOfRow();
            }
            std::cout << ft << std::endl;
        }

        // Clear Screen
        else if(input_line == "#clear"){
            //std::cout << std::string(50, '\n'); //From
            std::cout << "\033[H\033[J";//To this
        }

        // Parse as program
        else {

            // Add line to program
            global::program += input_line;

            // Count number of open scopes
            unsigned int open_scopes = 0;
            open_scopes += std::count(input_line.begin(), input_line.end(), '{');
            open_scopes -= std::count(input_line.begin(), input_line.end(), '}');

            while(open_scopes){
                std::cout << "... _\b";

                // Read next line
                input_line.clear();
                getline(std::cin, input_line);

                // Update scope count
                open_scopes += std::count(input_line.begin(), input_line.end(), '{');
                open_scopes -= std::count(input_line.begin(), input_line.end(), '}');

                // Add line to program
                global::program += input_line + "\n";
            }
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
            if (file_load)
                std::cout << "File loaded successfully." << std::endl;

            // If expression, show user output
            else if(expr){
                auto current = interpreter.current_expr();
                switch(current.first){
                    case parser::INT:
                        std::cout << current.second.i;
                        break;
                    case parser::REAL:
                        std::cout << current.second.r;
                        break;
                    case parser::BOOL:
                        std::cout << ((current.second.b) ? "true" : "false");
                        break;
                    case parser::STRING:
                        std::cout << current.second.s;
                        break;
                }
            }
        }

        // Catch exception and print error
        catch(const std::exception &e) {
            std::cerr << e.what() << std::endl;
        }
    //Clear global program
    global::program = "";
    }

    return 0;
}
void console_args(std::string fileargs){

    // Indefinite User input
    //for(;;){

    // Variables for user input
    std::string input_line;
    bool file_load = false;
    bool expr = false;
        // User prompt
        input_line = fileargs;

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
void help_menu(){
            std::cout << "\n" << "Welcome to \e[1;3;4mV3N0M "<<version<<"!\e[0m \n\n";
            std::cout << "When using the console the interpreter will pick the following as arguments:\n\n";
            std::cout << std::setw(18);
            std::cout << "--h or -h or --help or -help       For help menu.\n\n";
            std::cout << std::setw(18);
            std::cout << "Any other argument is reguaded as a \e[1;3;4mVENOM 1.0.0!\e[0m program file path \n\n";
            std::cout << std::setw(18);
            std::cout << "To use this interactive REPL, just type in regular VENOM commands and hit\n";
            std::cout << "enter. You can also make use of the following commands: \n\n";

            std::cout << " #load \e[1;3;4mfile-path\e[0m  ";
            std::cout << "Loads variable and function declarations from a specified\n";
            std::cout << std::setw(18);
            std::cout << "" << "file into memory, e.g.\n";
            std::cout << std::setw(18);
            std::cout << "" << ">>> #load ~/hello_world.prog\n\n";
            std::cout << " #quit            Exits the VENOM REPL.\n\n";
            std::cout << " #st              Displays the symbol table, a list of currently declared \n";
            std::cout << std::setw(18);
            std::cout << "" << "functions and variables in the global scope.\n\n";
            std::cout << " #clear           Clears the terminal window." << std::endl;
}
