#include "include/visitor.h"
#include "include/scope.h"
#include "include/value.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static ast_T* builtinFunctionPrint(visitor_T* visitor, ast_T** args, int argsSize) {
	
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
	
	printf("Created variable '%s' with value '%s'\n", node->variableDefinitionVariableName, value->stringValue);
	
	return (void*)0;
}

value_T* visitorVisitFunctionDefinition(visitor_T* visitor, ast_T* node) {
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
	
	return (void*)0;
}

value_T* visitorVisitString(visitor_T* visitor, ast_T* node) {
	return valueInitString(node->stringValue);
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
