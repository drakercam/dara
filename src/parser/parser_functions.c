#include "../include/parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/*
 * Parses a function definition, including its parameters, return
 * type, and body.
 *
 * Parameters:
 *   parser - Parser containing the current token stream.
 *   scope  - Scope in which the function is defined.
 *
 * Returns:
 *   The AST node representing the function definition.
 *
 * Terminates the program if the definition is invalid or an
 * allocation fails.
 */

ast_T* parserParseFunctionDefinition(parser_T* parser, scope_T* scope) {
	ast_T* ast = astInit(AST_FUNCTION_DEFINITION);
	
	parserEat(parser, TOKEN_ID);	// func keyword
	
	char* functionName = parser->currentToken->value;
	ast->functionDefinitionName = calloc(strlen(functionName) + 1, sizeof(char));
	strcpy(ast->functionDefinitionName, functionName);
	
	parserEat(parser, TOKEN_ID);	// func name
	
	parserEat(parser, TOKEN_LEFTPAREN);


    // Parse function parameters.
    if (parser->currentToken->type != TOKEN_RIGHTPAREN) {
        while (1) {

            char* parameterType = (void*)0;
            char* parameterName = (void*)0;
            bool isReference = false;
            bool isConstant = false;
            
            // An optional const qualifier precedes the parameter type.
			if (strcmp(parser->currentToken->value, "const") == 0) {
				isConstant = true;
				parserEat(parser, TOKEN_ID);
			}

			if (parser->currentToken->type != TOKEN_ID) {
				parserReportError(parser, "E1114", "Expected a parameter type.");
				astFree(ast);
				exit(EXIT_FAILURE);
			}

            char* firstName = parser->currentToken->value;

            // Built-in and generic parameter types.
            if (
                strcmp(firstName, "int") == 0 ||
                strcmp(firstName, "float") == 0 ||
                strcmp(firstName, "string") == 0 ||
                strcmp(firstName, "auto") == 0 ||
                strcmp(firstName, "bool") == 0 ||
                strcmp(firstName, "var") == 0
            ) {
                const char* typeName =
                    strcmp(firstName, "var") == 0 ? "auto" : firstName;

                parameterType = calloc(strlen(typeName) + 1, sizeof(char));

                if (parameterType != (void*)0) {
                    strcpy(parameterType, typeName);
                }

                parserEat(parser, TOKEN_ID);
            }
            else {
                // Construct types, including qualified names.
                parameterType = parserParseQualifiedConstructName(parser, scope);
            }

            if (parameterType == (void*)0) {
                printf("Failed to allocate function parameter type\n");
                astFree(ast);
                exit(EXIT_FAILURE);
            }

            // An ampersand marks an explicit reference parameter.
            if (parser->currentToken->type == TOKEN_AMPERSAND) {
                isReference = true;
                parserEat(parser, TOKEN_AMPERSAND);
            }

            // Every parameter must have a type followed by a name.
            if (parser->currentToken->type != TOKEN_ID) {
                parserReportError(parser, "E1115", "Expected a parameter name after the type.");
				astFree(ast);
				exit(EXIT_FAILURE);
            }

            parameterName = calloc(
                strlen(parser->currentToken->value) + 1,
                sizeof(char)
            );

            if (parameterName == (void*)0) {
                printf("Failed to allocate function parameter name\n");
                free(parameterType);
                astFree(ast);
                exit(EXIT_FAILURE);
            }

            strcpy(parameterName, parser->currentToken->value);
            parserEat(parser, TOKEN_ID);

            ast_T* parameter = astInit(AST_VARIABLE_DEFINITION);
            parameter->variableDefinitionType = parameterType;
            parameter->variableDefinitionVariableName = parameterName;
            parameter->variableDefinitionValue = (void*)0;
            parameter->variableDefinitionIsReference = isReference;
            parameter->variableDefinitionIsConstant = isConstant;
            parameter->scope = scope;

            size_t newSize = ast->functionDefinitionArgsSize + 1;

            ast_T** newArgs = realloc(
                ast->functionDefinitionArgs,
                newSize * sizeof(ast_T*)
            );

            if (newArgs == (void*)0) {
                printf("Failed to allocate function parameters\n");
                astFree(parameter);
                astFree(ast);
                exit(EXIT_FAILURE);
            }

            ast->functionDefinitionArgs = newArgs;
            ast->functionDefinitionArgs[newSize - 1] = parameter;
            ast->functionDefinitionArgsSize = newSize;

            if (parser->currentToken->type != TOKEN_COMMA) {
                break;
            }

            parserEat(parser, TOKEN_COMMA);
        }
    }
	
	parserEat(parser, TOKEN_RIGHTPAREN);

	// Parse the return type: -> int, -> float, -> string, -> noval
	parserEat(parser, TOKEN_ARROW);

	if (parser->currentToken->type != TOKEN_ID) {
		parserReportError(parser, "E1116", "Expected a function return type after '->'.");
		astFree(ast);
		exit(1);
	}

	char* returnType = (void*)0;
	const char* typeName = parser->currentToken->value;
	
	
	bool isBuiltinType = strcmp(typeName, "int") == 0 ||
						 strcmp(typeName, "float") == 0 ||
						 strcmp(typeName, "string") == 0 ||
						 strcmp(typeName, "bool") == 0 ||
						 strcmp(typeName, "noval") == 0 ||
						 strcmp(typeName, "construct") == 0 ||
						 strcmp(typeName, "color") == 0 ||
						 strcmp(typeName, "auto") == 0;
						 
	if (isBuiltinType) {
		returnType = calloc(strlen(typeName) + 1, sizeof(char));
		
		if (!returnType) {
			printf("Failed to allocate function return type\n");
			astFree(ast);
			exit(EXIT_FAILURE);
		}
		
		strcpy(returnType, typeName);
		parserEat(parser, TOKEN_ID);
	}
	else {
		// parse construct types, including qualified construct names.
		returnType = parserParseQualifiedConstructName(parser, scope);
	}
	
	if (!returnType) {
		astFree(ast);
		exit(1);
	}
	
	bool isConstructReturnType = scopeGetQualifiedConstructDefinition(scope, returnType) != (void*)0;
	
	if (!isBuiltinType && !isConstructReturnType) {
		char message[256];

		snprintf(
			message,
			sizeof(message),
			"Unknown function return type '%s'.",
			returnType
		);

		parserReportError(parser, "E1117", message);
		free(returnType);
		astFree(ast);
		exit(EXIT_FAILURE);
	}
	
	ast->functionDefinitionReturnType = returnType;
	
	parserEat(parser, TOKEN_LEFTBRACE);
	ast->functionDefinitionBody = parserParseStatements(parser, scope); // the function body is a compound
	parserEat(parser, TOKEN_RIGHTBRACE);
	
	ast->scope = scope;
	
	return ast;
}

/*
 * Parses a return statement and its optional return expression.
 *
 * Parameters:
 *   parser - Parser containing the current token stream.
 *   scope  - Scope used to parse the return expression.
 *
 * Returns:
 *   The AST node representing the return statement.
 */

ast_T* parserParseReturnStatement(parser_T* parser, scope_T* scope) {
	ast_T* returnStatement = astInit(AST_RETURN);
	parserSetNodeLocation(returnStatement, parser->currentToken);
	
	parserEat(parser, TOKEN_ID);	// return
	
	returnStatement->scope = scope;
	
	if (parser->currentToken->type != TOKEN_SEMI &&
		parser->currentToken->type != TOKEN_RIGHTBRACE) {
		returnStatement->returnValue = parserParseComparison(parser, scope);
	}
	
	return returnStatement;
}
