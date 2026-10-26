#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"
#include "scope.h"

typedef struct PARSER_STRUCT {
	lexer_T* lexer;
	token_T* currentToken;
	token_T* previousToken;
	scope_T* scope;
	
	const char* fileName;
	
} parser_T;

parser_T* parserInit(lexer_T* lexer);
void parserFree(parser_T* parser);
void parserEat(parser_T* parser, int tokenType); // we expect a certain token, if not we die

ast_T* parserParse(parser_T* parser, scope_T* scope);	// return our source code as an AST tree

void parserSetNodeLocation(ast_T* node, const token_T* token);
void parserReportError(parser_T* parser, const char* code, const char* message);

/*
 * Parses an if statement, including optional else branches.
 */
ast_T* parserParseIfStatement(parser_T* parser, scope_T* scope);

/*
 * Parses a while loop.
 */
ast_T* parserParseWhileLoop(parser_T* parser, scope_T* scope);

/*
 * Parses a for loop.
 */
ast_T* parserParseForLoop(parser_T* parser, scope_T* scope);


/*
 * Parses an assignment and its target expression.
 */
ast_T* parserParseAssignment(parser_T* parser, scope_T* scope);

/*
 * Parses a variable reference or an unqualified function call.
 */
ast_T* parserParseVariable(parser_T* parser, scope_T* scope);

/*
 * Parses a variable declaration, including its type and initializer.
 */
ast_T* parserParseVariableDefinition(parser_T* parser, scope_T* scope);

ast_T* parserParseStatement(parser_T* parser, scope_T* scope);
ast_T* parserParseStatements(parser_T* parser, scope_T* scope);
ast_T* parserParseFactor(parser_T* parser, scope_T* scope);

/*
 * Parses a std:array constructor expression.
 */
ast_T* parserParseArrayConstructor(parser_T* parser, scope_T* scope, sourceLocation_T startLocation);

/*
 * Parses addition and subtraction expressions.
 */
ast_T* parserParseExpression(parser_T* parser, scope_T* scope);

/*
 * Parses multiplication and division expressions.
 */
ast_T* parserParseTerm(parser_T* parser, scope_T* scope);

/*
 * Parses comparison expressions, including equality and
 * relational operators.
 */
ast_T* parserParseComparison(parser_T* parser, scope_T* scope);

/*
 * Parses unary expressions, including logical negation.
 */
ast_T* parserParseUnary(parser_T* parser, scope_T* scope);

/*
 * Parses a function definition, including its parameters, return
 * type, and body.
 *
 * Returns the AST node representing the function definition.
 */
ast_T* parserParseFunctionDefinition(parser_T* parser, scope_T* scope);

/*
 * Parses a return statement and its optional return expression.
 *
 * Returns the AST node representing the return statement.
 */
ast_T* parserParseReturnStatement(parser_T* parser, scope_T* scope);

/*
 * Parses a namespace declaration, including its body, and registers
 * the namespace definition in the supplied scope.
 *
 * Parameters:
 *   parser - Parser containing the current token stream.
 *   scope  - Scope in which the namespace is declared.
 *
 * Returns:
 *   The AST node representing the namespace definition.
 */
ast_T* parserParseNamespaceDefinition(parser_T* parser, scope_T* scope);

/*
 * Parses and registers a construct definition, including its
 * fields and methods.
 *
 * Returns the AST node representing the definition.
 */
ast_T* parserParseConstructDefinition(parser_T* parser, scope_T* scope);

/*
 * Parses a construct instantiation and its designated field
 * initializers.
 *
 * Returns the AST node representing the instantiation.
 */
ast_T* parserParseConstructInstantiation(parser_T* parser, scope_T* scope);

/*
 * Parses a potentially namespace-qualified construct name and
 * validates that it resolves in the supplied scope.
 *
 * Returns a dynamically allocated name string that the caller
 * must free.
 */
char* parserParseQualifiedConstructName(parser_T* parser, scope_T* scope);


/*
 * Parses a string literal.
 */
ast_T* parserParseString(parser_T* parser, scope_T* scope);

/*
 * Parses a numeric literal.
 */
ast_T* parserParseNumber(parser_T* parser, scope_T* scope);

/*
 * Parses a potentially namespace-qualified function name.
 *
 * Returns an allocated name string that the caller must free.
 */
char* parserParseQualifiedFunctionName(parser_T* parser);

/*
 * Parses a function call using an already parsed function name.
 */
ast_T* parserParseFunctionCallNamed(parser_T* parser, scope_T* scope, const char* functionName);

/*
 * Parses an unqualified function call.
 */
ast_T* parserParseFunctionCall(parser_T* parser, scope_T* scope);


ast_T* parserParseID(parser_T* parser, scope_T* scope);

#endif
