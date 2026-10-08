#include "include/scope.h"

#include <stdlib.h>
#include <string.h>

scope_T* scopeInit() {
	scope_T* scope = calloc(1, sizeof(struct SCOPE_STRUCT));
	scope->functionDefinitions = (void*)0;
	scope->functionDefinitionsSize = 0;
	
	scope->variableDefinitions = (void*)0;
	scope->variableDefinitionsSize = 0;
	
	return scope;
}

void scopeFree(scope_T* scope) {
	if (scope == (void*)0) {
		return;
	}
	
	free(scope->functionDefinitions);
	
	free(scope->variableDefinitions);
	
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

ast_T* scopeAddVariableDefinition(scope_T* scope, ast_T* varDef) {
	if (scope->variableDefinitions == (void*)0) {
		scope->variableDefinitions = calloc(1, sizeof(ast_T*));
		scope->variableDefinitions[0] = varDef;
		scope->variableDefinitionsSize += 1;
	}
	else {
		scope->variableDefinitionsSize += 1;
		scope->variableDefinitions = realloc(
			scope->variableDefinitions,
			scope->variableDefinitionsSize * sizeof(ast_T*)
		);
		scope->variableDefinitions[scope->variableDefinitionsSize-1] = varDef;
	}
	
	return varDef;
}

ast_T* scopeGetVariableDefinition(scope_T* scope, const char* varName) {
	for (size_t i = scope->variableDefinitionsSize; i-- > 0;) {
		ast_T* varDef = scope->variableDefinitions[i];

		if (strcmp(varDef->variableDefinitionVariableName, varName) == 0) {
			return varDef;
		}
	}
	
	return (void*)0;
}

void scopeRemoveVariableDefinitions(scope_T* scope, size_t count)
{
    for (size_t i = 0; i < count; ++i) {
        size_t index = scope->variableDefinitionsSize - 1;

        astFreeVariableDefinition(scope->variableDefinitions[index]);

        scope->variableDefinitionsSize -= 1;
    }

    if (scope->variableDefinitionsSize == 0) {
        free(scope->variableDefinitions);
        scope->variableDefinitions = (void*)0;
    }
    else {
        scope->variableDefinitions = realloc(
            scope->variableDefinitions,
            scope->variableDefinitionsSize * sizeof(ast_T*)
        );
    }
}
