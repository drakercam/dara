#include "include/visitor.h"
#include "include/scope.h"
#include "include/value.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static value_T* builtinFunctionPrint(visitor_T* visitor, ast_T** args, int argsSize) {
	for (int i = 0; i < argsSize; ++i) {
		value_T* value = visitorVisit(visitor, args[i]);
		
		if (value == (void*)0) {
			continue;
		}
		
		switch (value->type) {
			
			case VALUE_NULL:
				printf("null\n");
				break;
				
			case VALUE_NUMBER:
				printf("%g\n", value->numberValue);
				break;
			
			case VALUE_STRING:
				printf("%s\n", value->stringValue);
				break;
				
			case VALUE_STRUCT:
				printf("<struct>\n");
				break;
		}
		
		valueFree(value);
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

value_T* visitorVisit(visitor_T* visitor, ast_T* node) {
	
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
			
		case AST_NUMBER:
			return visitorVisitNumber(visitor, node);
			break;
			
		case AST_COMPOUND:
			return visitorVisitCompound(visitor, node);
			break;
			
		case AST_NOOP:
			return (void*)0;
			
	}
	
	printf("Uncaught statement of type '%d'\n", node->type);
	return (void*)0;
}

value_T* visitorVisitVariableDefinition(visitor_T* visitor, ast_T* node) {
	value_T* value = visitorVisit(visitor, node->variableDefinitionValue);
	
	scopeAddVariable(node->scope, node->variableDefinitionVariableName, value);
	
	//printf("Created variable '%s' with value '%s'\n", node->variableDefinitionVariableName, value->stringValue);
	
	return (void*)0;
}

value_T* visitorVisitFunctionDefinition(visitor_T* visitor, ast_T* node) {
	
	scopeAddFunctionDefinition(node->scope, node);
	
	return (void*)0;
}

value_T* visitorVisitVariable(visitor_T* visitor, ast_T* node) {
		
	variable_T* var = scopeGetVariable(node->scope, node->variableName);
				
	if (var != (void*)0) {
		return valueCopy(var->value);
	}
	
	printf("Undefined variable '%s'\n", node->variableName);
	exit(1);
}

value_T* visitorVisitFunctionCall(visitor_T* visitor, ast_T* node) {
	if (strcmp(node->functionCallName, "print") == 0) {
		return builtinFunctionPrint(
			visitor,
			node->functionCallArguments,
			node->functionCallArgumentsSize
		);
	}
	
	ast_T* funcDef = scopeGetFunctionDefinition(
		node->scope,
		node->functionCallName
	);

	if (funcDef == (void*)0) {
		printf("Undefined function '%s'\n", node->functionCallName);
		exit(1);
	}

	if (funcDef->functionDefinitionArgsSize != node->functionCallArgumentsSize) {
		printf(
			"ERROR: Calling function '%s' with %zu argument(s)\n"
			"-- Expects %zu argument(s)\n",
			funcDef->functionDefinitionName,
			node->functionCallArgumentsSize,
			funcDef->functionDefinitionArgsSize
		);
		exit(1);
	}
	
	// going through the function call arguments and adding the arguments to the functionbody's scope
	for (size_t i = 0; i < funcDef->functionDefinitionArgsSize; ++i) {
		ast_T* parameter = funcDef->functionDefinitionArgs[i];

		value_T* argument = visitorVisit(
			visitor,
			node->functionCallArguments[i]
		);

		scopeAddVariable(
			funcDef->functionDefinitionBody->scope,
			parameter->variableName,
			argument
		);
	}

	visitorVisit(visitor, funcDef->functionDefinitionBody);

	scopeRemoveVariables(
		funcDef->functionDefinitionBody->scope,
		funcDef->functionDefinitionArgsSize
	);
}

value_T* visitorVisitString(visitor_T* visitor, ast_T* node) {
	return valueInitString(node->stringValue);
}

value_T* visitorVisitNumber(visitor_T* visitor, ast_T* node) {
	return valueInitNumber(node->numberValue);
}

value_T* visitorVisitCompound(visitor_T* visitor, ast_T* node) {
	value_T* result = (void*)0;
	
	for (size_t i = 0; i < node->compoundSize; ++i) {
		result = visitorVisit(visitor, node->compoundValue[i]);
	}
	
	// returns the value of its last statement, which will be useful for 
	// implementing return/expression handling
	return result;
}
