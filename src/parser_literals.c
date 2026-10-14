#include "include/parser.h"

#include <stdlib.h>
#include <string.h>

/*
 * Parses a string literal and stores its value in a string AST node.
 *
 * Parameters:
 *   parser - Parser containing the current string token.
 *   scope  - Scope associated with the parsed expression.
 *
 * Returns:
 *   The AST node representing the string literal.
 */

ast_T* parserParseString(parser_T* parser, scope_T* scope) {
	
	ast_T* astString = astInit(AST_STRING);
	astString->stringValue = calloc(strlen(parser->currentToken->value) + 1, sizeof(char));
    strcpy(astString->stringValue, parser->currentToken->value);
	
	parserEat(parser, TOKEN_STRING);
	
	astString->scope = scope;
	
	return astString;
}

/*
 * Parses a numeric literal and determines whether it is an integer
 * or floating-point value.
 *
 * Parameters:
 *   parser - Parser containing the current numeric token.
 *   scope  - Scope associated with the parsed expression.
 *
 * Returns:
 *   The AST node representing the numeric literal.
 */

ast_T* parserParseNumber(parser_T* parser, scope_T* scope) {
	ast_T* astNumber = astInit(AST_NUMBER);
	astNumber->numberIsFloat = strchr(parser->currentToken->value, '.') != (void*)0;
	astNumber->numberValue = strtod(parser->currentToken->value, (void*)0);
	
	parserEat(parser, TOKEN_NUMBER);
	
	astNumber->scope = scope;
	
	return astNumber;
}
