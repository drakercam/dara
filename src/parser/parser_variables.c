#include "../include/parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * Parses an assignment, including its target and assigned expression.
 *
 * Parameters:
 *   parser - Parser containing the current token stream.
 *   scope  - Scope used to parse the assignment.
 *
 * Returns:
 *   The AST node representing the assignment.
 *
 * Terminates the program if the assignment target is invalid.
 */
ast_T* parserParseAssignment(parser_T* parser, scope_T* scope) {
    ast_T* ast = astInit(AST_ASSIGNMENT);
    ast->scope = scope;

    // Parse the complete left-hand side.
    ast->assignmentTarget = parserParseFactor(parser, scope);

    // Only variables and member accesses can be assignment targets.
    if (ast->assignmentTarget->type != AST_VARIABLE &&
        ast->assignmentTarget->type != AST_MEMBER_ACCESS) {
        parserReportError(parser, "E1103", "Invalid Assignment Target.");
        astFree(ast);
        exit(EXIT_FAILURE);
    }

    // Record the assignment operator.
    ast->assignmentOperator = parser->currentToken->type;

    if (ast->assignmentOperator != TOKEN_EQUALS &&
        ast->assignmentOperator != TOKEN_PLUS_EQUAL &&
        ast->assignmentOperator != TOKEN_MINUS_EQUAL) {
        parserReportError(parser, "E1104", "Expected an Assignment Operator.");
        astFree(ast);
        exit(EXIT_FAILURE);
    }

    parserEat(parser, ast->assignmentOperator);

    // Parse the right-hand expression.
    ast->assignmentValue = parserParseComparison(parser, scope);

    return ast;
}

/*
 * Parses a variable reference or an unqualified function call.
 *
 * Parameters:
 *   parser - Parser containing the current token stream.
 *   scope  - Scope used to resolve the variable or call.
 *
 * Returns:
 *   The AST node representing the variable or function call.
 */
 
 ast_T* parserParseVariable(parser_T* parser, scope_T* scope) {
	
	char* tokenValue = parser->currentToken->value;
	parserEat(parser, TOKEN_ID); // var name or function call name
	
	if (parser->currentToken->type == TOKEN_LEFTPAREN) {
		return parserParseFunctionCall(parser, scope);
	}
		
	ast_T* astVariable = astInit(AST_VARIABLE);
	parserSetNodeLocation(astVariable, parser->currentToken);
	astVariable->variableName = calloc(strlen(tokenValue) + 1, sizeof(char));
	strcpy(astVariable->variableName, tokenValue);
	
	astVariable->scope = scope;
	
	return astVariable;
}

/*
 * Parses a variable declaration, including its type, name,
 * and initializer expression.
 *
 * Parameters:
 *   parser - Parser containing the current token stream.
 *   scope  - Scope in which the variable is declared.
 *
 * Returns:
 *   The AST node representing the variable definition.
 */

ast_T* parserParseVariableDefinition(parser_T* parser, scope_T* scope) {
	char* variableDefinitionType = (void*)0;

	if (
		strcmp(parser->currentToken->value, "var") == 0 ||
		strcmp(parser->currentToken->value, "auto") == 0 ||
		strcmp(parser->currentToken->value, "int") == 0 ||
		strcmp(parser->currentToken->value, "float") == 0 ||
		strcmp(parser->currentToken->value, "string") == 0 ||
		strcmp(parser->currentToken->value, "color") == 0 ||
		strcmp(parser->currentToken->value, "bool") == 0
	) {
		const char* typeName = parser->currentToken->value;

		variableDefinitionType = calloc(
			strlen(typeName) + 1,
			sizeof(char)
		);

		if (variableDefinitionType == (void*)0) {
			printf("Failed to allocate variable type\n");
			exit(EXIT_FAILURE);
		}

		if (strcmp(typeName, "var") == 0) {
			strcpy(variableDefinitionType, "auto");
		} else {
			strcpy(variableDefinitionType, typeName);
		}

		parserEat(parser, TOKEN_ID);
	} else {
		variableDefinitionType =
			parserParseQualifiedConstructName(parser, scope);
	}
	char* variableName = parser->currentToken->value;
	char* variableDefinitionVariableName = calloc(strlen(parser->currentToken->value) + 1, sizeof(char));
	strcpy(variableDefinitionVariableName, variableName);
	
	sourceLocation_T startLocation = parser->currentToken->location;
	parserEat(parser, TOKEN_ID); // var name
	
	ast_T* variableDefinition = astInit(AST_VARIABLE_DEFINITION);
	variableDefinition->variableDefinitionType = variableDefinitionType;
	variableDefinition->variableDefinitionVariableName = variableDefinitionVariableName;
	variableDefinition->variableDefinitionValue = (void*)0;
	variableDefinition->scope = scope;
	variableDefinition->location = startLocation;
	
	if (parser->currentToken->type == TOKEN_EQUALS) {
		parserEat(parser, TOKEN_EQUALS);
		
		variableDefinition->variableDefinitionValue = parserParseComparison(parser, scope);
	}
	
	return variableDefinition;	
}
