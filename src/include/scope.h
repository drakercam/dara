#ifndef SCOPE_H
#define SCOPE_H

#include "ast.h"
#include "value.h"

typedef struct VARIABLE_STRUCT {
	
	char* name;
	value_T* value;
	
}variable_T;

typedef struct SCOPE_STRUCT {
	
	ast_T** functionDefinitions;
	size_t functionDefinitionsSize;
	
	variable_T** variables;
	size_t variablesSize;
	
}scope_T;

scope_T* scopeInit();
void scopeFree(scope_T* scope);

ast_T* scopeAddFunctionDefinition(scope_T* scope, ast_T* funcDef);
ast_T* scopeGetFunctionDefinition(scope_T* scope, const char* funcName);

variable_T* scopeAddVariable(scope_T* scope, const char* name, value_T* value);
variable_T* scopeGetVariable(scope_T* scope, const char* name);
void scopeRemoveVariables(scope_T* scope, size_t count);

ast_T* scopeAddVariableDefinition(scope_T* scope, ast_T* varDef);
ast_T* scopeGetVariableDefinition(scope_T* scope, const char* varName);
void scopeRemoveVariableDefinitions(scope_T* scope, size_t count);

#endif
