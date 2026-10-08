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
	
} parser_T;

parser_T* parserInit(lexer_T* lexer);
void parserFree(parser_T* parser);
void parserEat(parser_T* parser, int tokenType); // we expect a certain token, if not we die

ast_T* parserParse(parser_T* parser, scope_T* scope);	// return our source code as an AST tree
ast_T* parserParseStatement(parser_T* parser, scope_T* scope);
ast_T* parserParseStatements(parser_T* parser, scope_T* scope);
ast_T* parserParseExpression(parser_T* parser, scope_T* scope);
ast_T* parserParseFactor(parser_T* parser, scope_T* scope);
ast_T* parserParseTerm(parser_T* parser, scope_T* scope);
ast_T* parserParseFunctionCall(parser_T* parser, scope_T* scope);
ast_T* parserParseVariable(parser_T* parser, scope_T* scope);
ast_T* parserParseVariableDefinition(parser_T* parser, scope_T* scope);
ast_T* parserParseFunctionDefinition(parser_T* parser, scope_T* scope);
ast_T* parserParseString(parser_T* parser, scope_T* scope);

ast_T* parserParseID(parser_T* parser, scope_T* scope);

#endif
