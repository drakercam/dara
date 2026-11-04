#include "include/scope.h"
#include "include/ast.h"

#include <stdlib.h>
#include <string.h>

scope_T* scopeInit() {
	scope_T* scope = calloc(1, sizeof(struct SCOPE_STRUCT));
	
	if (!scope) {
        return NULL;
    }
	
	scope->callables = (void*)0;
	scope->callablesSize = 0;
	
	scope->variables = (void*)0;
	scope->variablesSize = 0;
	
	scope->constructDefinitions = (void*)0;
	scope->constructDefinitionsSize = 0;
	
	scope->namespaceDefinitions = (void*)0;
	scope->namespaceDefinitionsSize = 0;
	
	return scope;
}

scope_T* scopeInitChild(scope_T* parent) {
	if (!parent) {
        return NULL;
    }
	
	scope_T* scope = scopeInit();
	
	if (!scope) {
        return NULL;
    }
	
    scope->parent = parent;

    return scope;
}

void scopeFree(scope_T* scope) {
	if (scope == (void*)0) {
		return;
	}
	
	free(scope->constructDefinitions);
	free(scope->namespaceDefinitions);
	
	for (size_t i = 0; i < scope->variablesSize; ++i) {
		variable_T* variable = scope->variables[i];

		free(variable->name);
		if (variable->ownsValue) {
			valueFree(variable->value);
		}
		free(variable);
	}
	
	for (size_t i = 0; i < scope->callablesSize; ++i) {
		callable_T* callable = scope->callables[i];

		free(callable->name);
		free(callable);
	}

	free(scope->callables);
	free(scope->variables);
	free(scope);
}

/*
 * Checks whether a callable with the given name already exists
 * in the supplied scope.
 */
static callable_T* scopeFindLocalCallable(scope_T* scope, const char* qualifiedName) {
    for (size_t i = 0; i < scope->callablesSize; ++i) {
        callable_T* callable = scope->callables[i];

        if (strcmp(callable->name, qualifiedName) == 0) {
            return callable;
        }
    }

    return NULL;
}

/*
 * Registers a callable in the supplied scope.
 *
 * The registry takes ownership of the callable record and its name,
 * but never takes ownership of the implementation itself.
 */
static callable_T* scopeRegisterCallable(scope_T* scope, const char* qualifiedName,callableType_T type) {
    if (scope == NULL || qualifiedName == NULL || qualifiedName[0] == '\0') {
        return NULL;
    }

    if (scopeFindLocalCallable(scope, qualifiedName) != NULL) {
        return NULL;
    }

    callable_T* callable = calloc(1, sizeof(callable_T));

    if (callable == NULL) {
        return NULL;
    }

    callable->name = malloc(strlen(qualifiedName) + 1);

    if (callable->name == NULL) {
        free(callable);
        return NULL;
    }

    strcpy(callable->name, qualifiedName);
    callable->type = type;

    callable_T** callables = realloc(
        scope->callables,
        (scope->callablesSize + 1) * sizeof(callable_T*)
    );

    if (callables == NULL) {
        free(callable->name);
        free(callable);
        return NULL;
    }

    scope->callables = callables;
    scope->callables[scope->callablesSize] = callable;
    scope->callablesSize++;

    return callable;
}

/*
 * Registers a native C function.
 */
callable_T* scopeRegisterNativeFunction(scope_T* scope, const char* qualifiedName, nativeFunction_T callback) {
    if (callback == NULL) {
        return NULL;
    }

    callable_T* callable = scopeRegisterCallable(
        scope,
        qualifiedName,
        CALLABLE_NATIVE
    );

    if (callable == NULL) {
        return NULL;
    }

    callable->implementation.nativeFunction = callback;

    return callable;
}

/*
 * Registers a Dara AST function.
 *
 * The AST remains owned by the program's AST tree.
 */
callable_T* scopeRegisterDaraFunction(scope_T* scope, const char* qualifiedName, ast_T* functionDefinition) {
    if (functionDefinition == NULL) {
        return NULL;
    }

    callable_T* callable = scopeRegisterCallable(
        scope,
        qualifiedName,
        CALLABLE_DARA
    );

    if (callable == NULL) {
        return NULL;
    }

    callable->implementation.functionDefinition = functionDefinition;

    return callable;
}

/*
 * Searches the current scope and its parent scopes for a callable.
 */
callable_T* scopeGetCallable(scope_T* scope,const char* qualifiedName) {
    if (scope == NULL || qualifiedName == NULL) {
        return NULL;
    }

    scope_T* currentScope = scope;

    while (currentScope != NULL) {
        callable_T* callable = scopeFindLocalCallable(
            currentScope,
            qualifiedName
        );

        if (callable != NULL) {
            return callable;
        }

        currentScope = currentScope->parent;
    }

    return NULL;
}

static variable_T* scopeAddVariableWithOwnership(scope_T* scope, const char* name, value_T* value, bool ownsValue) {
    variable_T* var = calloc(1, sizeof(variable_T));

    if (var == (void*)0) {
        return (void*)0;
    }

    var->name = calloc(strlen(name) + 1, sizeof(char));

    if (var->name == (void*)0) {
        free(var);
        return (void*)0;
    }

    strcpy(var->name, name);

    var->value = value;
    var->ownsValue = ownsValue;

    variable_T** variables = realloc(
        scope->variables,
        (scope->variablesSize + 1) * sizeof(variable_T*)
    );

    if (variables == (void*)0) {
        free(var->name);
        free(var);
        return (void*)0;
    }

    scope->variables = variables;
    scope->variables[scope->variablesSize] = var;
    scope->variablesSize++;

    return var;
}

variable_T* scopeAddVariable(scope_T* scope, const char* name, value_T* value) {
    return scopeAddVariableWithOwnership(
        scope, name, value, true
    );
}

variable_T* scopeAddReferenceVariable(scope_T* scope, const char* name, value_T* value) {
    return scopeAddVariableWithOwnership(
        scope, name, value, false
    );
}

variable_T* scopeGetVariable(scope_T* scope, const char* name) {
    scope_T* currentScope = scope;

    while (currentScope != (void*)0) {
        for (size_t i = currentScope->variablesSize; i-- > 0;) {
            variable_T* variable = currentScope->variables[i];

            if (strcmp(variable->name, name) == 0) {
                return variable;
            }
        }

        currentScope = currentScope->parent;
    }

    return (void*)0;
}

variable_T* scopeAddConstant(scope_T* scope, const char* name, value_T* value) {
	variable_T* var = scopeAddVariableWithOwnership(scope, name, value, true);
	
	if (var) {
		var->isReadOnly = true;
	}
	
	return var;
}

void scopeRemoveVariables(scope_T* scope, size_t count) {
	if (scope == (void*)0) {
		return;
	}
	
	if (count > scope->variablesSize) {
		count = scope->variablesSize;
	}
	
	for (size_t i = 0; i < count; ++i) {
		
		size_t index = scope->variablesSize - 1;

		variable_T* variable = scope->variables[index];

		free(variable->name);
		if (variable->ownsValue) {
			valueFree(variable->value);
		}
		free(variable);

		scope->variablesSize -= 1;
	}

	if (scope->variablesSize == 0) {
		free(scope->variables);
		scope->variables = (void*)0;
	}
	else {
		variable_T** variables = realloc(
			scope->variables,
			scope->variablesSize * sizeof(variable_T*)
		);

		if (variables != NULL) {
			scope->variables = variables;
		}
	}
}

ast_T* scopeAddConstructDefinition(scope_T* scope, ast_T* constructDef) {
	scope->constructDefinitions = realloc(scope->constructDefinitions, 
								  (scope->constructDefinitionsSize + 1) * sizeof(ast_T*)
	);
	
	if (scope->constructDefinitions == (void*)0) {
		return (void*)0;
	}
	
	scope->constructDefinitions[scope->constructDefinitionsSize] = constructDef;
	
	scope->constructDefinitionsSize += 1;
	
	return constructDef;
}

ast_T* scopeGetConstructDefinition(scope_T* scope, const char* constructName) {
	if (!constructName) {
		return (void*)0;
	}
	
	scope_T* currentScope = scope;
	
	while (currentScope != (void*)0) {
		for (size_t i = 0; i < currentScope->constructDefinitionsSize; ++i) {
			ast_T* constructDef = currentScope->constructDefinitions[i];
			
			if (strcmp(constructDef->constructDefinitionName, constructName) == 0) {
				return constructDef;
			}
		}
		
		currentScope = currentScope->parent;
	}
	
	return (void*)0;
}

ast_T* scopeAddNamespaceDefinition(scope_T* scope, ast_T* namespaceDef) {
    ast_T** definitions = realloc(
        scope->namespaceDefinitions,
        (scope->namespaceDefinitionsSize + 1) * sizeof(ast_T*)
    );

    if (definitions == (void*)0) {
        return NULL;
    }

    scope->namespaceDefinitions = definitions;

    scope->namespaceDefinitions[scope->namespaceDefinitionsSize] =
        namespaceDef;

    scope->namespaceDefinitionsSize++;

    return namespaceDef;
}

ast_T* scopeGetNamespaceDefinition(scope_T* scope, const char* namespaceName) {
	scope_T* currentScope = scope;
	
    while (currentScope != (void*)0) {
		for (size_t i = 0; i < currentScope->namespaceDefinitionsSize; ++i) {
			ast_T* namespaceDef = currentScope->namespaceDefinitions[i];

			if (strcmp(namespaceDef->namespaceDefinitionName, namespaceName) == 0) {
				return namespaceDef;
			}
		}
		
		currentScope = currentScope->parent;
	}

    return (void*)0;
}


ast_T* scopeGetQualifiedConstructDefinition( scope_T* scope, const char* qualifiedName) {
    if (scope == (void*)0 || qualifiedName == (void*)0 ||
        qualifiedName[0] == '\0') {
        return (void*)0;
    }

    scope_T* currentScope = scope;
    const char* name = qualifiedName;
    const char* separator;

    while ((separator = strchr(name, ':')) != (void*)0) {
        size_t namespaceNameLength = separator - name;

        // Reject empty namespace segments, e.g. std::array.
        if (namespaceNameLength == 0) {
            return (void*)0;
        }

        char* namespaceName = calloc(
            namespaceNameLength + 1,
            sizeof(char)
        );

        if (namespaceName == (void*)0) {
            return (void*)0;
        }

        memcpy(namespaceName, name, namespaceNameLength);

        ast_T* namespaceDefinition = scopeGetNamespaceDefinition(currentScope, namespaceName);

        free(namespaceName);

        if (namespaceDefinition == (void*)0 ||
            namespaceDefinition->namespaceDefinitionScope == (void*)0) {
            return (void*)0;
        }

        // Continue searching inside this namespace's scope.
        currentScope = namespaceDefinition->namespaceDefinitionScope;
        name = separator + 1;

        if (*name == '\0') {
            return (void*)0;
        }
    }

    // The final segment is the construct name.
    return scopeGetConstructDefinition(currentScope, name);
}
