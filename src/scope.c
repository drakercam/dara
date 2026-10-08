#include "include/scope.h"

#include <stdlib.h>
#include <string.h>

scope_T* scopeInit() {
	scope_T* scope = calloc(1, sizeof(struct SCOPE_STRUCT));
	
	scope->functionDefinitions = (void*)0;
	scope->functionDefinitionsSize = 0;
	
	scope->variables = (void*)0;
	scope->variablesSize = 0;
	
	return scope;
}

void scopeFree(scope_T* scope) {
	if (scope == (void*)0) {
		return;
	}
	
	free(scope->functionDefinitions);
	
	for (size_t i = 0; i < scope->variablesSize; ++i) {
		variable_T* variable = scope->variables[i];

		free(variable->name);
		valueFree(variable->value);
		free(variable);
	}

	free(scope->variables);
	free(scope);
}

ast_T* scopeAddFunctionDefinition(scope_T* scope, ast_T* funcDef) {
	scope->functionDefinitionsSize += 1;
	
	if (scope->functionDefinitions == (void*)0) {
		scope->functionDefinitions = calloc(1, sizeof(ast_T*));
	}
	else {
		scope->functionDefinitions = 
			realloc(
				scope->functionDefinitions, 
				scope->functionDefinitionsSize * sizeof(ast_T**)
			);
	}
	
	scope->functionDefinitions[scope->functionDefinitionsSize-1] = funcDef;
	
	return funcDef;
}

ast_T* scopeGetFunctionDefinition(scope_T* scope, const char* funcName) {
	for (int i = 0; i < scope->functionDefinitionsSize; ++i) {
		ast_T* funcDef = scope->functionDefinitions[i];
		if (strcmp(funcDef->functionDefinitionName, funcName) == 0) {
			return funcDef;
		}
	}
	
	return (void*)0;
}

variable_T* scopeAddVariable(scope_T* scope, const char* name, value_T* value) {
	variable_T* var = calloc(1, sizeof(variable_T));
	
	var->name = calloc(strlen(name) + 1, sizeof(char));
	strcpy(var->name, name);
	
	var->value = value;
	
	scope->variablesSize += 1;
	
	scope->variables =
		realloc(
			scope->variables,
			scope->variablesSize * sizeof(variable_T*)
		);

	scope->variables[scope->variablesSize - 1] = var;

	return var;
}

variable_T* scopeGetVariable(scope_T* scope, const char* name) {
	for (size_t i = scope->variablesSize; i-- > 0;) {
		variable_T* variable = scope->variables[i];

		if (strcmp(variable->name, name) == 0) {
			return variable;
		}
	}

	return (void*)0;
}

void scopeRemoveVariables(scope_T* scope, size_t count) {
	for (size_t i = 0; i < count; ++i) {
		
		size_t index = scope->variablesSize - 1;

		variable_T* variable = scope->variables[index];

		free(variable->name);
		valueFree(variable->value);
		free(variable);

		scope->variablesSize -= 1;
	}

	if (scope->variablesSize == 0) {
		free(scope->variables);
		scope->variables = (void*)0;
	}
	else {
		scope->variables = realloc(
				scope->variables,
				scope->variablesSize * sizeof(variable_T*)
		);
	}
}
