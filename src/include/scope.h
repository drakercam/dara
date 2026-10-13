#ifndef SCOPE_H
#define SCOPE_H

#include "value.h"

typedef struct AST_STRUCT ast_T;

typedef struct VARIABLE_STRUCT {
	
	char* name;
	value_T* value;
	bool ownsValue;
	
}variable_T;

typedef struct SCOPE_STRUCT {
	
	struct SCOPE_STRUCT* parent;
	
	// function defs
	ast_T** functionDefinitions;
	size_t functionDefinitionsSize;
	
	// runtime vars
	variable_T** variables;
	size_t variablesSize;
	
	// construct defs
	ast_T** constructDefinitions;
	size_t constructDefinitionsSize;
	
	// namespace defs
	ast_T** namespaceDefinitions;
	size_t namespaceDefinitionsSize;
	
}scope_T;

scope_T* scopeInit();
scope_T* scopeInitChild(scope_T* parent);
void scopeFree(scope_T* scope);

ast_T* scopeAddFunctionDefinition(scope_T* scope, ast_T* funcDef);
ast_T* scopeGetFunctionDefinition(scope_T* scope, const char* funcName);

variable_T* scopeAddVariable(scope_T* scope, const char* name, value_T* value);
variable_T* scopeAddReferenceVariable(scope_T* scope, const char* name, value_T* value);
variable_T* scopeGetVariable(scope_T* scope, const char* name);
void scopeRemoveVariables(scope_T* scope, size_t count);

ast_T* scopeAddVariableDefinition(scope_T* scope, ast_T* varDef);
ast_T* scopeGetVariableDefinition(scope_T* scope, const char* varName);
void scopeRemoveVariableDefinitions(scope_T* scope, size_t count);

ast_T* scopeAddConstructDefinition(scope_T* scope, ast_T* constructDef);
ast_T* scopeGetConstructDefinition(scope_T* scope, const char* constructName);

ast_T* scopeAddNamespaceDefinition(scope_T* scope, ast_T* namespaceDef);
ast_T* scopeGetNamespaceDefinition(scope_T* scope, const char* namespaceName);

ast_T* scopeGetQualifiedConstructDefinition(scope_T* scope, const char* qualifiedName);
ast_T* scopeGetQualifiedFunctionDefinition(scope_T* scope, const char* qualifiedName);

#endif
