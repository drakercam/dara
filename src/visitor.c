#include "include/visitor.h"
#include "include/scope.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static ast_T* builtinFunctionPrint(visitor_T* visitor, ast_T** args, int argsSize) {
	for (int i = 0; i < argsSize; ++i) {
		ast_T* visitedAst = visitorVisit(visitor, args[i]);
		
		switch(visitedAst->type) {
			
			case AST_STRING:
				printf("%s\n", visitedAst->stringValue);
				//return (void*)0;
				//break;
		}
		
		//printf("%p\n", visitedAst);
	}
	
	return (void*)0;
}

visitor_T* visitorInit() {
	visitor_T* visitor = calloc(1, sizeof(visitor_T));
	
	return visitor;
}

void visitorFree(visitor_T* visitor) {
	if (visitor == (void*)0) {
		return;
	}
	
	free(visitor);
}

ast_T* visitorVisit(visitor_T* visitor, ast_T* node) {
	
	//printf("type=%d\n", node->type);
		
	switch(node->type) {
		
		case AST_VARIABLE_DEFINITION:
			return visitorVisitVariableDefinition(visitor, node);
			break;
		
		case AST_FUNCTION_DEFINITION:
			return visitorVisitFunctionDefinition(visitor, node);
			break;
		
		case AST_VARIABLE:
			return visitorVisitVariable(visitor, node);
			break;
			
		case AST_FUNCTION_CALL:
			return visitorVisitFunctionCall(visitor, node);
			break;
			
		case AST_STRING:
			return visitorVisitString(visitor, node);
			break;
			
		case AST_COMPOUND:
			return visitorVisitCompound(visitor, node);
			break;
			
		case AST_NOOP:
			return node;
			break;
			
	}
	
	printf("Uncaught statement of type '%d'\n", node->type);
	return astInit(AST_NOOP);
}

ast_T* visitorVisitVariableDefinition(visitor_T* visitor, ast_T* node) {
	scopeAddVariableDefinition(node->scope, node);
	
	return node;
}

ast_T* visitorVisitFunctionDefinition(visitor_T* visitor, ast_T* node) {
	scopeAddFunctionDefinition(node->scope, node);
	
	return node;
}

ast_T* visitorVisitVariable(visitor_T* visitor, ast_T* node) {
		
	ast_T* varDef = scopeGetVariableDefinition(node->scope, node->variableName);
		
	if (varDef != (void*)0) {
		return visitorVisit(visitor, varDef->variableDefinitionValue);
	}
	
	printf("Undefined variable '%s'\n", node->variableName);
	exit(1);
}

ast_T* visitorVisitFunctionCall(visitor_T* visitor, ast_T* node) {
	
	if (strcmp(node->functionCallName, "print") == 0) {
		return builtinFunctionPrint(visitor, node->functionCallArguments, node->functionCallArgumentsSize);
	}
	
	ast_T* funcDef = scopeGetFunctionDefinition(node->scope, node->functionCallName);
	
	if (funcDef == (void*)0) {
		printf("Undefined method '%s'\n", node->functionCallName);
		exit(1);
	}
	
	if (funcDef->functionDefinitionArgsSize != node->functionCallArgumentsSize) {
		printf("ERROR: Calling function: '%s' with '%d' argument(s)\n-- Expects '%d' argument(s)\n",
					funcDef->functionDefinitionName,
					node->functionCallArgumentsSize,
					funcDef->functionDefinitionArgsSize
		);
		exit(1);
	}
	
	for (int i = 0; i < funcDef->functionDefinitionArgsSize; ++i) {
		// grab the variable from the function definition arguments
		ast_T* astVar = (ast_T*)funcDef->functionDefinitionArgs[i];
		
		// grab the value from the function call arguments
		ast_T* astValue = (ast_T*)node->functionCallArguments[i];
		
		// create variable definition
		ast_T* varDef = astInit(AST_VARIABLE_DEFINITION);
		
		// copy variable name into variable definition
		varDef->variableDefinitionVariableName = (char*)calloc(strlen(astVar->variableName) + 1, sizeof(char));
		strcpy(varDef->variableDefinitionVariableName, astVar->variableName);
		
		// attach value to variable definition
		varDef->variableDefinitionValue = astValue;
		
		scopeAddVariableDefinition(funcDef->functionDefinitionBody->scope, varDef);
	}
	
	ast_T* result = visitorVisit(visitor, funcDef->functionDefinitionBody);
	scopeRemoveVariableDefinitions(
		funcDef->functionDefinitionBody->scope,
		funcDef->functionDefinitionArgsSize
	);
		
	return result;
}

ast_T* visitorVisitString(visitor_T* visitor, ast_T* node) {
	
	return node;
}

ast_T* visitorVisitCompound(visitor_T* visitor, ast_T* node) {
	for (int i = 0; i < node->compoundSize; ++i) {
		visitorVisit(visitor, node->compoundValue[i]);
	}
	
	return (void*)0;
}
