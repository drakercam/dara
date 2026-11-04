
#ifndef SCOPE_H
#define SCOPE_H

#include "value.h"

typedef struct AST_STRUCT ast_T;
typedef struct VISITOR_STRUCT visitor_T;

/*
 * Native functions use a common callback signature.
 *
 * Arguments are AST nodes so native functions can evaluate them
 * through the visitor, preserving existing evaluation behavior.
 */
typedef value_T* (*nativeFunction_T)(
    visitor_T* visitor,
    ast_T** args,
    int argsSize
);

/*
 * Identifies how a callable is implemented.
 */
typedef enum CALLABLE_TYPE {
    CALLABLE_DARA,
    CALLABLE_NATIVE
} callableType_T;

/*
 * Represents any invocable function in Dara.
 *
 * Dara callables reference an AST function definition.
 * Native callables reference a C implementation.
 */
typedef struct CALLABLE_STRUCT {
    char* name;
    callableType_T type;

    union {
        ast_T* functionDefinition;
        nativeFunction_T nativeFunction;
    } implementation;

} callable_T;

typedef struct VARIABLE_STRUCT {

    char* name;
    value_T* value;
    bool ownsValue;
    bool isReadOnly;

} variable_T;

typedef struct SCOPE_STRUCT {

    struct SCOPE_STRUCT* parent;

    // Callable registry
    callable_T** callables;
    size_t callablesSize;

    // Runtime variables
    variable_T** variables;
    size_t variablesSize;

    // Construct definitions
    ast_T** constructDefinitions;
    size_t constructDefinitionsSize;

    // Namespace definitions
    ast_T** namespaceDefinitions;
    size_t namespaceDefinitionsSize;

} scope_T;

scope_T* scopeInit();
scope_T* scopeInitChild(scope_T* parent);
void scopeFree(scope_T* scope);

/*
 * Callable registry
 */
callable_T* scopeRegisterNativeFunction(scope_T* scope, const char* qualifiedName, nativeFunction_T callback);

callable_T* scopeRegisterDaraFunction(scope_T* scope, const char* qualifiedName, ast_T* functionDefinition);

callable_T* scopeGetCallable(scope_T* scope, const char* qualifiedName);

variable_T* scopeAddVariable(scope_T* scope, const char* name, value_T* value);
variable_T* scopeAddReferenceVariable(scope_T* scope, const char* name, value_T* value);
variable_T* scopeGetVariable(scope_T* scope, const char* name);
variable_T* scopeAddConstant(scope_T* scope, const char* name, value_T* value);

void scopeRemoveVariables(scope_T* scope, size_t count);

ast_T* scopeAddVariableDefinition(scope_T* scope, ast_T* varDef);
ast_T* scopeGetVariableDefinition(scope_T* scope, const char* varName);
void scopeRemoveVariableDefinitions(scope_T* scope, size_t count);

ast_T* scopeAddConstructDefinition(scope_T* scope, ast_T* constructDef);
ast_T* scopeGetConstructDefinition(scope_T* scope, const char* constructName);

ast_T* scopeAddNamespaceDefinition(scope_T* scope, ast_T* namespaceDef);
ast_T* scopeGetNamespaceDefinition(scope_T* scope, const char* namespaceName);

ast_T* scopeGetQualifiedConstructDefinition(scope_T* scope, const char* qualifiedName);

#endif
