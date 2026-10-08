#ifndef SCOPE_H
#define SCOPE_H

#include "ast.h"

typedef struct SCOPE_STRUCT {
	
	ast_T** functionDefinitions;
	size_t functionDefinitionsSize;
	
	ast_T** variableDefinitions;
	size_t variableDefinitionsSize;
	
}scope_T;

scope_T* scopeInit();
void scopeFree(scope_T* scope);

ast_T* scopeAddFunctionDefinition(scope_T* scope, ast_T* funcDef);
ast_T* scopeGetFunctionDefinition(scope_T* scope, const char* funcName);

ast_T* scopeAddVariableDefinition(scope_T* scope, ast_T* varDef);
ast_T* scopeGetVariableDefinition(scope_T* scope, const char* varName);
void scopeRemoveVariableDefinitions(scope_T* scope, size_t count);

#endif
