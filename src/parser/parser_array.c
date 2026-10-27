#include "../include/parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * 	Parses an array constructor
 */

ast_T* parserParseArrayConstructor(parser_T* parser, scope_T* scope, sourceLocation_T startLocation) {
	ast_T* arrayConstructor = astInit(AST_ARRAY_CONSTRUCTOR);
	arrayConstructor->scope = scope;
	arrayConstructor->location = startLocation;

	parserEat(parser, TOKEN_LEFTPAREN);

	// The capacity is optional. Omitted capacity defaults to 1.
	if (parser->currentToken->type != TOKEN_RIGHTPAREN) {
		arrayConstructor->arrayConstructorCapacity =
			parserParseComparison(parser, scope);
	}

	parserEat(parser, TOKEN_RIGHTPAREN);
	parserEat(parser, TOKEN_ARROW);

	if (parser->currentToken->type != TOKEN_ID) {
		parserReportError(parser, "E1102", "Expected an element type after '->' in array constructor.");
		astFree(arrayConstructor);
		exit(EXIT_FAILURE);
	}

	const char* elementType = parser->currentToken->value;

	arrayConstructor->arrayConstructorElementType = calloc(strlen(elementType) + 1, sizeof(char));

	if (arrayConstructor->arrayConstructorElementType == (void*)0) {
		astFree(arrayConstructor);
		fprintf(stderr, "Failed to allocate array element type.\n");
		exit(1);
	}

	strcpy(arrayConstructor->arrayConstructorElementType, elementType);

	parserEat(parser, TOKEN_ID);

	return arrayConstructor;
}
