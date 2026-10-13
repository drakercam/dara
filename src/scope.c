#include "include/scope.h"
#include "include/ast.h"

#include <stdlib.h>
#include <string.h>

scope_T* scopeInit() {
	scope_T* scope = calloc(1, sizeof(struct SCOPE_STRUCT));
	
	scope->functionDefinitions = (void*)0;
	scope->functionDefinitionsSize = 0;
	
	scope->variables = (void*)0;
	scope->variablesSize = 0;
	
	scope->constructDefinitions = (void*)0;
	scope->constructDefinitionsSize = 0;
	
	scope->namespaceDefinitions = (void*)0;
	scope->namespaceDefinitionsSize = 0;
	
	return scope;
}

scope_T* scopeInitChild(scope_T* parent) {
	scope_T* scope = scopeInit();
    scope->parent = parent;

    return scope;
}

void scopeFree(scope_T* scope) {
	if (scope == (void*)0) {
		return;
	}
	
	free(scope->functionDefinitions);
	free(scope->constructDefinitions);
	free(scope->namespaceDefinitions);
	
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

    return NULL;
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
		valueFree(variable->value);
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
	for (size_t i = 0; i < scope->constructDefinitionsSize; ++i) {
		ast_T* constructDef = scope->constructDefinitions[i];
		
		if (strcmp(constructDef->constructDefinitionName, constructName) == 0) {
			return constructDef;
		}
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

ast_T* scopeGetNamespaceDefinition(
    scope_T* scope,
    const char* namespaceName
) {
    for (size_t i = 0; i < scope->namespaceDefinitionsSize; ++i) {
        ast_T* namespaceDef = scope->namespaceDefinitions[i];

        if (strcmp(
                namespaceDef->namespaceDefinitionName,
                namespaceName
            ) == 0) {
            return namespaceDef;
        }
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

        ast_T* namespaceDefinition =
            scopeGetNamespaceDefinition(currentScope, namespaceName);

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



