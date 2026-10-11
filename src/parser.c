#include "include/parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

parser_T* parserInit(lexer_T* lexer) {
	parser_T* parser = calloc(1, sizeof(parser_T));
	parser->lexer = lexer;
	parser->currentToken = lexerGetNextToken(lexer);
	parser->previousToken = (void*)0;
	
	parser->scope = scopeInit();
	
	return parser;
}

void parserFree(parser_T* parser) {
	if (parser == (void*)0) {
		return;
	}
	
	tokenFree(parser->previousToken);
	tokenFree(parser->currentToken);
	
	scopeFree(parser->scope);
	
	free(parser);
}

void parserEat(parser_T* parser, int tokenType) {
	if (parser->currentToken->type == tokenType) {
		tokenFree(parser->previousToken);
		
		parser->previousToken = parser->currentToken;
		parser->currentToken = lexerGetNextToken(parser->lexer);
	}
	else {
		printf("Unexpected token: '%s', with type %d\n", parser->currentToken->value, parser->currentToken->type);
		exit(1);
	}
}

ast_T* parserParse(parser_T* parser, scope_T* scope) {
	return parserParseStatements(parser, scope);
}

ast_T* parserParseStatement(parser_T* parser, scope_T* scope) {
	switch (parser->currentToken->type) {
		
		case TOKEN_ID:
			return parserParseID(parser, scope);
	}
	
	return astInit(AST_NOOP);
}

ast_T* parserParseStatements(parser_T* parser, scope_T* scope) {
	
	ast_T* compound = astInit(AST_COMPOUND);
	compound->scope = scope;
	compound->compoundValue = calloc(1, sizeof(ast_T*));
	
	ast_T* astStatement = parserParseStatement(parser, scope);
	astStatement->scope = scope;
	compound->compoundValue[0] = astStatement;
	compound->compoundSize += 1;
	
	while (parser->currentToken->type == TOKEN_SEMI) {
		parserEat(parser, TOKEN_SEMI);
				
		ast_T* astStatement = parserParseStatement(parser, scope);
		if (astStatement) {
			compound->compoundSize += 1;
			compound->compoundValue = realloc(compound->compoundValue, 
											  compound->compoundSize * sizeof(ast_T*));
			compound->compoundValue[compound->compoundSize-1] = astStatement;
		}
	}
	
	return compound;
}

ast_T* parserParseExpression(parser_T* parser, scope_T* scope) {
	ast_T* left = parserParseTerm(parser, scope);

	while (parser->currentToken->type == TOKEN_PLUS || parser->currentToken->type == TOKEN_MINUS) {
		int operationType = parser->currentToken->type;
		parserEat(parser, operationType);

		ast_T* right = parserParseTerm(parser, scope);

		ast_T* operation = astInit(AST_BINARY_OPERATION);
		operation->binaryOperationLeft = left;
		operation->binaryOperationRight = right;
		operation->binaryOperationType = operationType;
		operation->scope = scope;

		left = operation;
	}

	return left;
}

ast_T* parserParseFactor(parser_T* parser, scope_T* scope) {
	switch (parser->currentToken->type) {
		case TOKEN_STRING:
			return parserParseString(parser, scope);

		case TOKEN_NUMBER:
			return parserParseNumber(parser, scope);

		case TOKEN_ID:
			if (strcmp(parser->currentToken->value, "true") == 0 ||
				strcmp(parser->currentToken->value, "false") == 0) {
				ast_T* astBoolean = astInit(AST_BOOLEAN);
							
				astBoolean->booleanValue = strcmp(parser->currentToken->value, "true") == 0;
							
				astBoolean->scope = scope;
				
				parserEat(parser, TOKEN_ID);
							
				return astBoolean;
			}
		
			return parserParseID(parser, scope);
			
		case TOKEN_LEFTPAREN:
			parserEat(parser, TOKEN_LEFTPAREN);
			ast_T* expression = parserParseComparison(parser, scope);
			parserEat(parser, TOKEN_RIGHTPAREN);
			return expression;

		default:
			printf(
				"Unexpected token in expression: '%s', with type %d\n",
				parser->currentToken->value,
				parser->currentToken->type
			);
			exit(1);
	}
}

// handles multiplication and division which have a higher precedence than adding/subtracting
ast_T* parserParseTerm(parser_T* parser, scope_T* scope) {
	ast_T* left = parserParseUnary(parser, scope);

	while (parser->currentToken->type == TOKEN_MULTIPLY || parser->currentToken->type == TOKEN_DIVIDE) {
		int operationType = parser->currentToken->type;
		parserEat(parser, operationType);

		ast_T* right = parserParseUnary(parser, scope);

		ast_T* operation = astInit(AST_BINARY_OPERATION);
		operation->binaryOperationLeft = left;
		operation->binaryOperationRight = right;
		operation->binaryOperationType = operationType;
		operation->scope = scope;

		left = operation;
	}

	return left;
}

ast_T* parserParseFunctionCall(parser_T* parser, scope_T* scope) {
	//printf("func name: %s\n", parser->previousToken->value);
	ast_T* functionCall = astInit(AST_FUNCTION_CALL);
	
	functionCall->functionCallName = calloc(strlen(parser->previousToken->value) + 1, sizeof(char));
	strcpy(functionCall->functionCallName, parser->previousToken->value);
	
	parserEat(parser, TOKEN_LEFTPAREN);
	functionCall->functionCallArguments = calloc(1, sizeof(ast_T*));
	
	if (parser->currentToken->type != TOKEN_RIGHTPAREN) {
		ast_T* astExpression = parserParseComparison(parser, scope);
		functionCall->functionCallArguments[0] = astExpression;
		functionCall->functionCallArgumentsSize += 1;
		
		while (parser->currentToken->type == TOKEN_COMMA) {
			parserEat(parser, TOKEN_COMMA);
			
			ast_T* astExpression = parserParseComparison(parser, scope);
			functionCall->functionCallArgumentsSize += 1;
			functionCall->functionCallArguments = realloc(functionCall->functionCallArguments, 
											  functionCall->functionCallArgumentsSize * sizeof(ast_T*));
			functionCall->functionCallArguments[functionCall->functionCallArgumentsSize-1] = astExpression;
		}
	}
	
	parserEat(parser, TOKEN_RIGHTPAREN);
	
	functionCall->scope = scope;
	
	return functionCall;
}

ast_T* parserParseVariable(parser_T* parser, scope_T* scope) {
	
	char* tokenValue = parser->currentToken->value;
	parserEat(parser, TOKEN_ID); // var name or function call name
	
	if (parser->currentToken->type == TOKEN_LEFTPAREN) {
		return parserParseFunctionCall(parser, scope);
	}
		
	ast_T* astVariable = astInit(AST_VARIABLE);
	astVariable->variableName = calloc(strlen(tokenValue) + 1, sizeof(char));
	strcpy(astVariable->variableName, tokenValue);
	
	astVariable->scope = scope;
	
	return astVariable;
}

ast_T* parserParseVariableDefinition(parser_T* parser, scope_T* scope) {
	parserEat(parser, TOKEN_ID);	// var
	char* variableDefinitionVariableName = calloc(strlen(parser->currentToken->value) + 1, sizeof(char));
	strcpy(variableDefinitionVariableName, parser->currentToken->value);
	
	parserEat(parser, TOKEN_ID); // var name
	parserEat(parser, TOKEN_EQUALS);
	ast_T* variableDefinitionValue = parserParseComparison(parser, scope);
	
	ast_T* variableDefinition = astInit(AST_VARIABLE_DEFINITION);
	variableDefinition->variableDefinitionVariableName = variableDefinitionVariableName;
	variableDefinition->variableDefinitionValue = variableDefinitionValue;
	
	variableDefinition->scope = scope;
	
	return variableDefinition;
	
}

ast_T* parserParseReturnStatement(parser_T* parser, scope_T* scope) {
	parserEat(parser, TOKEN_ID);	// return
	ast_T* returnStatement = astInit(AST_RETURN);
	
	returnStatement->scope = scope;
	
	if (parser->currentToken->type != TOKEN_SEMI &&
		parser->currentToken->type != TOKEN_RIGHTBRACE) {
		returnStatement->returnValue = parserParseComparison(parser, scope);
	}
	
	return returnStatement;
}

ast_T* parserParseFunctionDefinition(parser_T* parser, scope_T* scope) {
	ast_T* ast = astInit(AST_FUNCTION_DEFINITION);
	
	parserEat(parser, TOKEN_ID);	// func keyword
	
	char* functionName = parser->currentToken->value;
	ast->functionDefinitionName = calloc(strlen(functionName) + 1, sizeof(char));
	strcpy(ast->functionDefinitionName, functionName);
	
	parserEat(parser, TOKEN_ID);	// func name
	
	parserEat(parser, TOKEN_LEFTPAREN);
	
	// this is where we handle function arguments
	if (parser->currentToken->type != TOKEN_RIGHTPAREN) {
		ast->functionDefinitionArgs = calloc(1, sizeof(ast_T*));
		ast_T* arg = parserParseVariable(parser, scope);
		ast->functionDefinitionArgsSize += 1;
		ast->functionDefinitionArgs[ast->functionDefinitionArgsSize-1] = arg;
			
		while (parser->currentToken->type == TOKEN_COMMA) {
			parserEat(parser, TOKEN_COMMA);
			
			ast->functionDefinitionArgsSize += 1;
					
			ast->functionDefinitionArgs = realloc(
						ast->functionDefinitionArgs,
						ast->functionDefinitionArgsSize * sizeof(ast_T*)
			);
			ast_T* arg = parserParseVariable(parser, scope);
			ast->functionDefinitionArgs[ast->functionDefinitionArgsSize-1] = arg;
		}
	}
	
	parserEat(parser, TOKEN_RIGHTPAREN);
	
	parserEat(parser, TOKEN_LEFTBRACE);
	ast->functionDefinitionBody = parserParseStatements(parser, scope); // the function body is a compound
	parserEat(parser, TOKEN_RIGHTBRACE);
	
	ast->scope = scope;
	
	return ast;
}

ast_T* parserParseString(parser_T* parser, scope_T* scope) {
	
	ast_T* astString = astInit(AST_STRING);
	astString->stringValue = calloc(strlen(parser->currentToken->value) + 1, sizeof(char));
    strcpy(astString->stringValue, parser->currentToken->value);
	
	parserEat(parser, TOKEN_STRING);
	
	astString->scope = scope;
	
	return astString;
}

ast_T* parserParseNumber(parser_T* parser, scope_T* scope) {
	ast_T* astNumber = astInit(AST_NUMBER);
	astNumber->numberValue = strtod(parser->currentToken->value, (void*)0);
	
	parserEat(parser, TOKEN_NUMBER);
	
	astNumber->scope = scope;
	
	return astNumber;
}

ast_T* parserParseComparison(parser_T* parser, scope_T* scope) {
	ast_T* left = parserParseExpression(parser, scope);

	while (parser->currentToken->type == TOKEN_EQUAL_EQUAL || 
		   parser->currentToken->type == TOKEN_LESS ||
		   parser->currentToken->type == TOKEN_MORE ||
		   parser->currentToken->type == TOKEN_LESS_THAN_EQUAL ||
		   parser->currentToken->type == TOKEN_MORE_THAN_EQUAL ||
		   parser->currentToken->type == TOKEN_NOT_EQUAL) {
		int operationType = parser->currentToken->type;
		parserEat(parser, operationType);

		ast_T* right = parserParseExpression(parser, scope);

		ast_T* operation = astInit(AST_BINARY_OPERATION);
		operation->binaryOperationLeft = left;
		operation->binaryOperationRight = right;
		operation->binaryOperationType = operationType;
		operation->scope = scope;

		left = operation;
	}

	return left;
}

ast_T* parserParseUnary(parser_T* parser, scope_T* scope) {
	if (parser->currentToken->type == TOKEN_NOT) {
		int operationType = parser->currentToken->type;
		parserEat(parser, TOKEN_NOT);
		
		ast_T* operand = parserParseUnary(parser, scope);
		
		ast_T* operation = astInit(AST_UNARY_OPERATION);
		operation->unaryOperationOperand = operand;
		operation->unaryOperationType = operationType;
		operation->scope = scope;
		
		return operation;
	}
	
	return parserParseFactor(parser, scope);
}

ast_T* parserParseID(parser_T* parser, scope_T* scope) {
	
	if (strcmp(parser->currentToken->value, "var") == 0) {
		return parserParseVariableDefinition(parser, scope);
	}
	else if (strcmp(parser->currentToken->value, "func") == 0) {
		return parserParseFunctionDefinition(parser, scope);
	}
	else if (strcmp(parser->currentToken->value, "return") == 0) {
		return parserParseReturnStatement(parser, scope);
	}
	else {
		return parserParseVariable(parser, scope);
	}
}
