#include "include/visitor.h"
#include "include/scope.h"
#include "include/value.h"
#include "include/builtin_functions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/*
 * Registers a function definition in its scope.
 */
value_T* visitorVisitFunctionDefinition(visitor_T* visitor, ast_T* node) {
	
	scopeAddFunctionDefinition(node->scope, node);
	
	return (void*)0;
}

/*
 * Registers function definitions contained within a namespace.
 */
value_T* visitorVisitNamespaceDefinition(visitor_T* visitor, ast_T* node) {
	// register the definitions inside the namespaces scope
	for (size_t i = 0; i < node->namespaceDefinitionBody->compoundSize; ++i) {
		ast_T* statement = node->namespaceDefinitionBody->compoundValue[i];
		
		if (statement->type == AST_FUNCTION_DEFINITION) {
			visitorVisitFunctionDefinition(visitor, statement);
		}
		else if (statement->type == AST_NAMESPACE_DEFINITION) {
			visitorVisitNamespaceDefinition(visitor, statement);
		}
	}
	
	return (void*)0;
}

/*
 * Evaluates a function call, binds arguments, and checks its return type.
 */
value_T* visitorVisitFunctionCall(visitor_T* visitor, ast_T* node) {
	if (strcmp(node->functionCallName, "std:print") == 0) {
		return builtinFunctionPrint(visitor, node->functionCallArguments, node->functionCallArgumentsSize);
	}
	
	if (strcmp(node->functionCallName, "std:add") == 0) {
		return builtinFunctionAdd(visitor, node->functionCallArguments, node->functionCallArgumentsSize);
	}
	
	// raylib
	if (strcmp(node->functionCallName, "rl:InitWindow") == 0) {
		return builtinRaylibFuncInitWindow(visitor, node->functionCallArguments, node->functionCallArgumentsSize);
	}
	
	if (strcmp(node->functionCallName, "rl:WindowShouldClose") == 0) {
		return builtinRaylibFuncWindowShouldClose(visitor, node->functionCallArguments, node->functionCallArgumentsSize);
	}
	
	if (strcmp(node->functionCallName, "rl:BeginDrawing") == 0) {
		return builtinRaylibFuncBeginDrawing(visitor, node->functionCallArguments, node->functionCallArgumentsSize);
	}
	
	if (strcmp(node->functionCallName, "rl:EndDrawing") == 0) {
		return builtinRaylibFuncEndDrawing(visitor, node->functionCallArguments, node->functionCallArgumentsSize);
	}
	
	if (strcmp(node->functionCallName, "rl:CloseWindow") == 0) {
		return builtinRaylibFuncCloseWindow(visitor, node->functionCallArguments, node->functionCallArgumentsSize);
	}
	
	if (strcmp(node->functionCallName, "rl:ClearBackground") == 0) {
		return builtinRaylibFuncClearBackground(visitor, node->functionCallArguments, node->functionCallArgumentsSize);
	}
	
	if (strcmp(node->functionCallName, "rl:DrawRectangle") == 0) {
		return builtinRaylibFuncDrawRectangle(visitor, node->functionCallArguments, node->functionCallArgumentsSize);
	}
	
	ast_T* funcDef;

	if (strchr(node->functionCallName, ':') != (void*)0) {
		funcDef = scopeGetQualifiedFunctionDefinition(
			node->scope,
			node->functionCallName
		);
	}
	else {
		funcDef = scopeGetFunctionDefinition(
			node->scope,
			node->functionCallName
		);
	}

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
	
	
	for (size_t i = 0; i < funcDef->functionDefinitionArgsSize; i++) {
		ast_T* parameter = funcDef->functionDefinitionArgs[i];
		ast_T* argumentNode = node->functionCallArguments[i];

		bool isReference = parameter->variableDefinitionIsReference;
		value_T* argument;

		if (isReference) {
			if (argumentNode->type != AST_VARIABLE &&
				argumentNode->type != AST_MEMBER_ACCESS) {
				printf(
					"Reference parameter '%s' requires a variable or field argument\n",
					parameter->variableDefinitionVariableName
				);
				exit(1);
			}

			argument = visitorGetAssignmentTargetValue(
				visitor,
				argumentNode
			);
		} else {
			argument = visitorVisit(visitor, argumentNode);
		}

		if (argument == (void*)0) {
			printf("Failed to evaluate function argument\n");
			exit(1);
		}

		if (!visitorArgumentMatchesType(parameter, argument)) {
			printf(
				"Argument type mismatch for parameter '%s'\n",
				parameter->variableDefinitionVariableName
			);

			if (!isReference) {
				valueFree(argument);
			}

			exit(1);
		}

		variable_T* binding;

		if (isReference) {
			binding = scopeAddReferenceVariable(
				funcDef->functionDefinitionBody->scope,
				parameter->variableDefinitionVariableName,
				argument
			);
		} else {
			binding = scopeAddVariable(
				funcDef->functionDefinitionBody->scope,
				parameter->variableDefinitionVariableName,
				argument
			);
		}

		if (binding == (void*)0) {
			if (!isReference) {
				valueFree(argument);
			}

			printf("Failed to bind function parameter\n");
			exit(1);
		}
	}

	visitor->shouldReturn = false;

	value_T* result = visitorVisit(visitor, funcDef->functionDefinitionBody);

	bool didReturn = visitor->shouldReturn;
	visitor->shouldReturn = false;


	if (!didReturn) {
		if (result != NULL) {
			valueFree(result);
		}

		result = valueInit(VALUE_NULL);
	}

	// Enforce the function's declared return type.
	char* returnType = funcDef->functionDefinitionReturnType;

	if (strcmp(returnType, "noval") == 0) {
		if (result->type != VALUE_NULL) {
			valueFree(result);
			printf(
				"Return type error: function '%s' declares noval\n",
				funcDef->functionDefinitionName
			);
			exit(1);
		}
	}
	else if (strcmp(returnType, "int") == 0) {
		if (!didReturn || result->type != VALUE_INT) {
			valueFree(result);
			printf(
				"Return type error: function '%s' expects int\n",
				funcDef->functionDefinitionName
			);
			exit(1);
		}
	}
	else if (strcmp(returnType, "float") == 0) {
		if (!didReturn || result->type != VALUE_FLOAT) {
			valueFree(result);
			printf(
				"Return type error: function '%s' expects float\n",
				funcDef->functionDefinitionName
			);
			exit(1);
		}
	}
	else if (strcmp(returnType, "string") == 0) {
		if (!didReturn || result->type != VALUE_STRING) {
			valueFree(result);
			printf(
				"Return type error: function '%s' expects string\n",
				funcDef->functionDefinitionName
			);
			exit(1);
		}
	}

	scopeRemoveVariables(
		funcDef->functionDefinitionBody->scope,
		funcDef->functionDefinitionArgsSize
	);
	
	return result;
}

/*
 * Evaluates a method call on a construct and manages the current method
 * and construct context.
 */
value_T* visitorVisitMethodCall(visitor_T* visitor, ast_T* node) {
    // 1. Find the construct definition.
    ast_T* construct = scopeGetQualifiedConstructDefinition(
        node->scope,
        node->methodCallConstructName
    );

    if (construct == (void*)0) {
        printf(
            "Undefined construct '%s'\n",
            node->methodCallConstructName
        );
        exit(1);
    }

    // 2. Find the requested method.
    ast_T* method = (void*)0;

    for (size_t i = 0;
         i < construct->constructDefinitionMethodsSize;
         ++i) {

        ast_T* candidate = construct->constructDefinitionMethods[i];

        if (strcmp(
                candidate->functionDefinitionName,
                node->methodCallName
            ) == 0) {
            method = candidate;
            break;
        }
    }

    if (method == (void*)0) {
        printf(
            "Construct '%s' has no method named '%s'\n",
            node->methodCallConstructName,
            node->methodCallName
        );
        exit(1);
    }

    // 3. Check the number of arguments.
    if (method->functionDefinitionArgsSize != node->methodCallArgumentsSize) {
        printf(
            "Method '%s.%s' expects %zu argument(s), but received %zu\n",
            node->methodCallConstructName,
            node->methodCallName,
            method->functionDefinitionArgsSize,
            node->methodCallArgumentsSize
        );
        exit(1);
    }

    // 4. Evaluate arguments and bind them to parameters.
	for (size_t i = 0; i < method->functionDefinitionArgsSize; i++) {
		ast_T* parameter = method->functionDefinitionArgs[i];
		ast_T* argumentNode = node->methodCallArguments[i];

		bool isReference = parameter->variableDefinitionIsReference;
		value_T* argument;

		if (isReference) {
			if (argumentNode->type != AST_VARIABLE &&
				argumentNode->type != AST_MEMBER_ACCESS) {
				printf(
					"Reference parameter '%s' requires a variable or field argument\n",
					parameter->variableDefinitionVariableName
				);
				exit(1);
			}

			argument = visitorGetAssignmentTargetValue(
				visitor,
				argumentNode
			);
		} else {
			argument = visitorVisit(visitor, argumentNode);
		}

		if (argument == (void*)0) {
			printf("Failed to evaluate method argument\n");
			exit(1);
		}

		if (!visitorArgumentMatchesType(parameter, argument)) {
			printf(
				"Argument type mismatch for parameter '%s'\n",
				parameter->variableDefinitionVariableName
			);

			if (!isReference) {
				valueFree(argument);
			}

			exit(1);
		}

		variable_T* binding;

		if (isReference) {
			binding = scopeAddReferenceVariable(
				method->functionDefinitionBody->scope,
				parameter->variableDefinitionVariableName,
				argument
			);
		} else {
			binding = scopeAddVariable(
				method->functionDefinitionBody->scope,
				parameter->variableDefinitionVariableName,
				argument
			);
		}

		if (binding == (void*)0) {
			if (!isReference) {
				valueFree(argument);
			}

			printf("Failed to bind method parameter\n");
			exit(1);
		}
	}
    
	ast_T* previousMethod = visitor->currentMethod;
	ast_T* previousConstruct = visitor->currentConstruct;

	visitor->currentMethod = method;
	visitor->currentConstruct = construct;

    // 5. Execute the method body.
    visitor->shouldReturn = false;

    value_T* result = visitorVisit(
        visitor,
        method->functionDefinitionBody
    );

    bool didReturn = visitor->shouldReturn;
    visitor->shouldReturn = false;

    // Match ordinary function behavior when no return was encountered.
    if (!didReturn) {
        valueFree(result);
        result = valueInit(VALUE_NULL);
    }
    
    visitor->currentMethod = previousMethod;
	visitor->currentConstruct = previousConstruct;

    // 6. Enforce the declared return type.
    char* returnType = method->functionDefinitionReturnType;

    bool validReturn =
        (strcmp(returnType, "noval") == 0 &&
         result->type == VALUE_NULL) ||
        (strcmp(returnType, "int") == 0 &&
         didReturn && result->type == VALUE_INT) ||
        (strcmp(returnType, "float") == 0 &&
         didReturn && result->type == VALUE_FLOAT) ||
        (strcmp(returnType, "string") == 0 &&
         didReturn && result->type == VALUE_STRING);

    if (!validReturn) {
        printf(
            "Return type error: method '%s.%s' declares '%s'\n",
            node->methodCallConstructName,
            node->methodCallName,
            returnType
        );

        valueFree(result);
        exit(1);
    }

    // 7. Remove the parameter variables from the method scope.
    scopeRemoveVariables(
        method->functionDefinitionBody->scope,
        method->functionDefinitionArgsSize
    );

    return result;
}

/*
 * Evaluates a return statement and signals that execution should return.
 */
value_T* visitorVisitReturn(visitor_T* visitor, ast_T* node) {
	value_T* result;
	
	if (node->returnValue == NULL) {
		result = valueInit(VALUE_NULL);
	}
	else {
		result = visitorVisit(visitor, node->returnValue);
	}
	
	visitor->shouldReturn = true;
	
	return result;
}
