/**
 * @file token.h
 * Contains the class Token, used to represent the tokens of the program string.
 * @author Morgan Okumu 
 * Carries the format of MINILANG A Lukec project in 2018
 * @updates were done to meet VENOM LANG stds
 */

#ifndef VENOM_TOKEN_H
#define VENOM_TOKEN_H

#include <string>
#include <iostream>
namespace lexer {

    /*
     * An enum containing token types.
     * This enum encodes the possible token types as described in table 1.3
     * in the report.
     */
    enum TOKEN {
        TOK_INT,
        TOK_REAL,
        TOK_ADDITIVE_OP,
        TOK_MULTIPLICATIVE_OP,
        TOK_RELATIONAL_OP,
        TOK_EQUALS,
        TOK_VAR,
        TOK_SET,
        TOK_DEF,
        TOK_RETURN,
        TOK_IF,
        TOK_ELSE,
        TOK_WHILE,
        TOK_PRINT,
        TOK_INT_TYPE,
        TOK_REAL_TYPE,
        TOK_BOOL_TYPE,
        TOK_STRING_TYPE,
        TOK_BOOL,
        TOK_NOT,
        TOK_IDENTIFIER,
        TOK_COMMENT,
        TOK_STRING,
        TOK_LEFT_CURLY,
        TOK_RIGHT_CURLY,
        TOK_LEFT_BRACKET,
        TOK_RIGHT_BRACKET,
        TOK_COMMA,
        TOK_SEMICOLON,
        TOK_COLON,
        TOK_EOF,
        TOK_ERROR,
        TOK_INCLUDE,
        TOK_FOR,
        TOK_RIGHT_SQUARE_BRACKET, 
        TOK_LEFT_SQUARE_BRACKET, 
        TOK_APPEND,
        TOK_PERIOD,
        TOK_INPUT
    };

    /**
     * Represents and individual token in the program.
     * This class stores the token type and value, as well as the line number in
     * which the token occurs (for error reporting).
     */
    class Token {

        public:
            Token();
            Token(int, std::string, unsigned int line_number = 0);
            TOKEN type;
            std::string value;
            unsigned int line_number;

        private:
            TOKEN determine_token_type(int, std::string&);
    };
};


#endif //VENOM_TOKEN_H
