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

static bool visitorBindArguments(visitor_T* visitor, ast_T* callNode, ast_T** parameters, size_t parameterCount, ast_T** arguments, size_t argumentCount, scope_T* targetScope, const char* callName) {
	if (parameterCount != argumentCount) {
		char message[256];

		snprintf(
			message,
			sizeof(message),
			"'%s' expects %zu argument(s), but received %zu",
			callName,
			parameterCount,
			argumentCount
		);

		visitorReportDiagnostic(visitor, callNode, "E2033", message);

		return false;
	}

	size_t boundCount = 0;

	for (size_t i = 0; i < parameterCount; ++i) {
		ast_T* parameter = parameters[i];
		ast_T* argumentNode = arguments[i];

		bool isReference = parameter->variableDefinitionIsReference;

		value_T* argument = (void*)0;

		if (isReference) {
			if (argumentNode->type != AST_VARIABLE && argumentNode->type != AST_MEMBER_ACCESS) {
				char message[256];

				snprintf(
					message,
					sizeof(message),
					"Reference parameter '%s' requires a variable or field argument",
					parameter->variableDefinitionVariableName
				);

				visitorReportDiagnostic(visitor, callNode, "E2036", message);

				scopeRemoveVariables(targetScope, boundCount);
				return false;
			}

			argument = visitorGetAssignmentTargetValue(visitor, argumentNode);
		}
		else {
			argument = visitorVisit(visitor, argumentNode);
		}

		/*
		 * The expression or assignment-target visitor is responsible
		 * for reporting its own error. Do not emit a duplicate here.
		 */
		if (argument == (void*)0) {
			scopeRemoveVariables(targetScope, boundCount);
			return false;
		}

		if (!visitorArgumentMatchesType(parameter, argument)) {
			char message[256];

			snprintf(
				message,
				sizeof(message),
				"Argument type mismatch for parameter '%s'",
				parameter->variableDefinitionVariableName
			);

			visitorReportDiagnostic(visitor, callNode, "E2037", message);

			if (!isReference) {
				valueFree(argument);
			}

			scopeRemoveVariables(targetScope, boundCount);
			return false;
		}

		variable_T* binding;

		if (isReference) {
			binding = scopeAddReferenceVariable(targetScope, parameter->variableDefinitionVariableName, argument);
		}
		else {
			binding = scopeAddVariable(targetScope, parameter->variableDefinitionVariableName, argument);
		}

		if (binding == (void*)0) {
			if (!isReference) {
				valueFree(argument);
			}

			visitorReportDiagnostic(visitor, callNode, "E2038", "Failed to bind a function or method parameter");

			scopeRemoveVariables(targetScope, boundCount);
			return false;
		}

		++boundCount;
	}

	return true;
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
		funcDef = scopeGetQualifiedFunctionDefinition(node->scope, node->functionCallName);
	}
	else {
		funcDef = scopeGetFunctionDefinition(node->scope, node->functionCallName);
	}

	if (!funcDef) {
		char message[256];
		
		snprintf(message, sizeof(message), "Undefined function '%s'.", node->functionCallName);
		
		visitorReportDiagnostic(visitor, node, "E2032", message);
		return (void*)0;
	}

	if (!visitorBindArguments(
		visitor,
		node,
		funcDef->functionDefinitionArgs,
		funcDef->functionDefinitionArgsSize,
		node->functionCallArguments,
		node->functionCallArgumentsSize,
		funcDef->functionDefinitionBody->scope,
		node->functionCallName)) 
	{
		return (void*)0;
	}
	
	
	bool previousShouldReturn = visitor->shouldReturn;
	visitor->shouldReturn = false;

	value_T* result = visitorVisit(visitor, funcDef->functionDefinitionBody);

	bool didReturn = visitor->shouldReturn;
	visitor->shouldReturn = false;


	if (!didReturn) {
		if (result != (void*)0) {
			valueFree(result);
		}

		result = valueInit(VALUE_NULL);
		
		if (!result) {
			scopeRemoveVariables(funcDef->functionDefinitionBody->scope, funcDef->functionDefinitionArgsSize);
			visitorReportDiagnostic(visitor, node, "E2040", "Failed to allocate a return value.");
			return (void*)0;
		}
	}
	
	/*
	 * If a return expression failed, visitorVisitReturn() should not
	 * set shouldReturn. Do not inspect a missing result as a value.
	 */
	if (!result) {
		scopeRemoveVariables(funcDef->functionDefinitionBody->scope, funcDef->functionDefinitionArgsSize);
		return (void*)0;
	}

	// Enforce the function's declared return type.
	char* returnType = funcDef->functionDefinitionReturnType;

	bool validReturn = false;

	if (strcmp(returnType, "noval") == 0) {
		validReturn = result->type == VALUE_NULL;
	}
	else if (strcmp(returnType, "int") == 0) {
		validReturn = result->type == VALUE_INT;
	}
	else if (strcmp(returnType, "float") == 0) {
		validReturn = result->type == VALUE_FLOAT;
	}
	else if (strcmp(returnType, "string") == 0) {
		validReturn = result->type == VALUE_STRING;
	}
	else if (strcmp(returnType, "array") == 0) {
		validReturn = result->type == VALUE_ARRAY;
	}

	if (!validReturn) {
		char message[256];

		snprintf(
			message,
			sizeof(message),
			"Function '%s' has an invalid return value for declared type '%s'",
			funcDef->functionDefinitionName,
			returnType
		);

		valueFree(result);

		scopeRemoveVariables(funcDef->functionDefinitionBody->scope, funcDef->functionDefinitionArgsSize);

		visitorReportDiagnostic(visitor, node, "E2039", message);

		return (void*)0;
	}

	scopeRemoveVariables(funcDef->functionDefinitionBody->scope, funcDef->functionDefinitionArgsSize);

	return result;
}

/*
 * Evaluates a method call on a construct and manages the current method
 * and construct context.
 */
value_T* visitorVisitMethodCall(visitor_T* visitor, ast_T* node) {
	/*
	 * 	Handle std library funcs first
	 */
	
	if (strcmp(node->methodCallConstructName, "std:array") == 0) {
		// dispatch to std:array builtins
		
		return visitorVisitArrayMethodCall(visitor, node);
	}
	
	
    // 1. Find the construct definition.
    ast_T* construct = scopeGetQualifiedConstructDefinition(node->scope, node->methodCallConstructName);

    if (!construct) {
        char message[256];
        
        snprintf(
			message,
			sizeof(message),
			"Undefined construct '%s'.",
			node->methodCallConstructName
        );
        
        visitorReportDiagnostic(visitor, node, "E2034", message);
        
        return (void*)0;
    }

    // 2. Find the requested method.
    ast_T* method = (void*)0;

    for (size_t i = 0; i < construct->constructDefinitionMethodsSize; ++i) {
        ast_T* candidate = construct->constructDefinitionMethods[i];

        if (strcmp(candidate->functionDefinitionName, node->methodCallName) == 0) {
            method = candidate;
            break;
        }
    }

    if (!method) {
        char message[256];

		snprintf(
			message,
			sizeof(message),
			"Construct '%s' has no method named '%s'",
			node->methodCallConstructName,
			node->methodCallName
		);

		visitorReportDiagnostic(visitor, node, "E2035", message);

		return (void*)0;
    }

    char callName[256];

	snprintf(
		callName,
		sizeof(callName),
		"%s.%s",
		node->methodCallConstructName,
		node->methodCallName
	);

	if (!visitorBindArguments(
			visitor,
			node,
			method->functionDefinitionArgs,
			method->functionDefinitionArgsSize,
			node->methodCallArguments,
			node->methodCallArgumentsSize,
			method->functionDefinitionBody->scope,
			callName
		)) {
		return (void*)0;
	}
    
	ast_T* previousMethod = visitor->currentMethod;
	ast_T* previousConstruct = visitor->currentConstruct;
	bool previousShouldReturn = visitor->shouldReturn;

	visitor->currentMethod = method;
	visitor->currentConstruct = construct;

    visitor->shouldReturn = false;

    value_T* result = visitorVisit(visitor, method->functionDefinitionBody);

    bool didReturn = visitor->shouldReturn;
    
    visitor->currentMethod = previousMethod;
	visitor->currentConstruct = previousConstruct;
	visitor->shouldReturn = previousShouldReturn;

    if (!didReturn) {
		if (result != (void*)0) {
			valueFree(result);
		}

		result = valueInit(VALUE_NULL);

		if (!result) {
			scopeRemoveVariables(method->functionDefinitionBody->scope, method->functionDefinitionArgsSize);
			visitorReportDiagnostic(visitor, node, "E2040", "Failed to allocate a return value");
			return (void*)0;
		}
	}

	if (!result) {
		scopeRemoveVariables(method->functionDefinitionBody->scope, method->functionDefinitionArgsSize);
		return (void*)0;
	}

	char* returnType = method->functionDefinitionReturnType;

	bool validReturn = false;

	if (strcmp(returnType, "noval") == 0) {
		validReturn = result->type == VALUE_NULL;
	}
	else if (strcmp(returnType, "int") == 0) {
		validReturn = didReturn && result->type == VALUE_INT;
	}
	else if (strcmp(returnType, "float") == 0) {
		validReturn = didReturn && result->type == VALUE_FLOAT;
	}
	else if (strcmp(returnType, "string") == 0) {
		validReturn = didReturn && result->type == VALUE_STRING;
	}
	else if (strcmp(returnType, "array") == 0) {
		validReturn = didReturn && result->type == VALUE_ARRAY;
	}

	if (!validReturn) {
		char message[256];

		snprintf(
			message,
			sizeof(message),
			"Method '%s' has an invalid return value for declared type '%s'",
			callName,
			returnType
		);

		valueFree(result);

		scopeRemoveVariables(method->functionDefinitionBody->scope, method->functionDefinitionArgsSize);

		visitorReportDiagnostic(visitor, node, "E2039", message);

		return (void*)0;
	}

	scopeRemoveVariables(method->functionDefinitionBody->scope, method->functionDefinitionArgsSize);

	return result;
}

/*
 * Evaluates a return statement and signals that execution should return.
 */
value_T* visitorVisitReturn(visitor_T* visitor, ast_T* node) {
	value_T* result;
	
	if (node->returnValue == (void*)0) {
		result = valueInit(VALUE_NULL);
	}
	else {
		result = visitorVisit(visitor, node->returnValue);
	}
	
	if (!result) {
		return (void*)0;
	}
	
	visitor->shouldReturn = true;
	
	return result;
}
