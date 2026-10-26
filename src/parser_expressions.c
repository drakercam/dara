#include "include/parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * Parses addition and subtraction expressions.
 *
 * Parameters:
 *   parser - Parser containing the current token stream.
 *   scope  - Scope used to parse the expression.
 *
 * Returns:
 *   The AST node representing the expression.
 */

ast_T* parserParseExpression(parser_T* parser, scope_T* scope) {
	ast_T* left = parserParseTerm(parser, scope);

	while (parser->currentToken->type == TOKEN_PLUS || parser->currentToken->type == TOKEN_MINUS) {
		token_T* operationToken = parser->currentToken;
		int operationType = parser->currentToken->type;
		parserEat(parser, operationType);

		ast_T* right = parserParseTerm(parser, scope);

		ast_T* operation = astInit(AST_BINARY_OPERATION);
		parserSetNodeLocation(operation, operationToken);
		operation->binaryOperationLeft = left;
		operation->binaryOperationRight = right;
		operation->binaryOperationType = operationType;
		operation->scope = scope;

		left = operation;
	}

	return left;
}

/*
 * Parses multiplication and division expressions, which have
 * higher precedence than addition and subtraction.
 *
 * Parameters:
 *   parser - Parser containing the current token stream.
 *   scope  - Scope used to parse the expression.
 *
 * Returns:
 *   The AST node representing the term.
 */

ast_T* parserParseTerm(parser_T* parser, scope_T* scope) {
	ast_T* left = parserParseUnary(parser, scope);

	while (parser->currentToken->type == TOKEN_MULTIPLY || parser->currentToken->type == TOKEN_DIVIDE) {
		token_T* operationToken = parser->currentToken;
		int operationType = parser->currentToken->type;
		parserEat(parser, operationType);

		ast_T* right = parserParseUnary(parser, scope);

		ast_T* operation = astInit(AST_BINARY_OPERATION);
		parserSetNodeLocation(operation, operationToken);
		operation->binaryOperationLeft = left;
		operation->binaryOperationRight = right;
		operation->binaryOperationType = operationType;
		operation->scope = scope;

		left = operation;
	}

	return left;
}

/*
 * Parses comparison expressions, including equality and
 * relational operators.
 *
 * Parameters:
 *   parser - Parser containing the current token stream.
 *   scope  - Scope used to parse the comparison.
 *
 * Returns:
 *   The AST node representing the comparison expression.
 */

ast_T* parserParseComparison(parser_T* parser, scope_T* scope) {
	ast_T* left = parserParseExpression(parser, scope);

	while (parser->currentToken->type == TOKEN_EQUAL_EQUAL || 
		   parser->currentToken->type == TOKEN_LESS ||
		   parser->currentToken->type == TOKEN_MORE ||
		   parser->currentToken->type == TOKEN_LESS_THAN_EQUAL ||
		   parser->currentToken->type == TOKEN_MORE_THAN_EQUAL ||
		   parser->currentToken->type == TOKEN_NOT_EQUAL) {
		token_T* operationToken = parser->currentToken;
		int operationType = parser->currentToken->type;
		parserEat(parser, operationType);

		ast_T* right = parserParseExpression(parser, scope);

		ast_T* operation = astInit(AST_BINARY_OPERATION);
		parserSetNodeLocation(operation, operationToken);
		operation->binaryOperationLeft = left;
		operation->binaryOperationRight = right;
		operation->binaryOperationType = operationType;
		operation->scope = scope;

		left = operation;
	}

	return left;
}

/*
 * Parses unary expressions, including logical negation.
 *
 * Parameters:
 *   parser - Parser containing the current token stream.
 *   scope  - Scope used to parse the expression.
 *
 * Returns:
 *   The AST node representing the unary expression or its operand.
 */

ast_T* parserParseUnary(parser_T* parser, scope_T* scope) {
	if (parser->currentToken->type == TOKEN_NOT) {
		token_T* operationToken = parser->currentToken;
		int operationType = parser->currentToken->type;
		parserEat(parser, TOKEN_NOT);
		
		ast_T* operand = parserParseUnary(parser, scope);
		
		ast_T* operation = astInit(AST_UNARY_OPERATION);
		parserSetNodeLocation(operation, operationToken);
		operation->unaryOperationOperand = operand;
		operation->unaryOperationType = operationType;
		operation->scope = scope;
		
		return operation;
	}
	
	return parserParseFactor(parser, scope);
}
