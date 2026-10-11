#include "include/visitor.h"
#include "include/scope.h"
#include "include/value.h"
#include "include/token.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static value_T* builtinFunctionPrint(visitor_T* visitor, ast_T** args, int argsSize) {
	for (int i = 0; i < argsSize; ++i) {
		value_T* value = visitorVisit(visitor, args[i]);
		
		if (value == (void*)0) {
			continue;
		}
		
		switch (value->type) {
			
			case VALUE_NULL:
				printf("noval\n");
				break;
				
			case VALUE_NUMBER:
				printf("%g\n", value->numberValue);
				break;
				
			case VALUE_BOOLEAN:
				printf("%s\n", value->booleanValue ? "true" : "false");
				break;
			
			case VALUE_STRING:
				printf("%s\n", value->stringValue);
				break;
				
			case VALUE_STRUCT:
				printf("<struct>\n");
				break;
		}
		
		valueFree(value);
	}
	
	return (void*)0;
}

static value_T* builtinFunctionAdd(visitor_T* visitor, ast_T** args, int argsSize) {
	int result = 0;
	
	for (int i = 0; i < argsSize; ++i) {
		value_T* value = visitorVisit(visitor, args[i]);
		
		if (value == (void*)0) {
			continue;
		}
		
		switch (value->type) {
				
			case VALUE_NUMBER:
				result += value->numberValue;
			
			default:
				printf("ERROR::BUILTIN::ADD: Expected a number\n");
				exit(1);
		}
		
		valueFree(value);
	}
	
	return (void*)0;
}

static value_T* builtinFunctionSub(visitor_T* visitor, ast_T** args, int argsSize) {
	
}

static value_T* builtinFunctionMultiply(visitor_T* visitor, ast_T** args, int argsSize) {
	
}

static value_T* builtinFunctionDivide(visitor_T* visitor, ast_T** args, int argsSize) {
	
}

visitor_T* visitorInit() {
	visitor_T* visitor = calloc(1, sizeof(visitor_T));
	
	return visitor;
}

void visitorFree(visitor_T* visitor) {
	if (visitor == (void*)0) {
		return;
	}
	
	free(visitor);
}

value_T* visitorVisit(visitor_T* visitor, ast_T* node) {
	
	//printf("type=%d\n", node->type);
		
	switch(node->type) {
		
		case AST_VARIABLE_DEFINITION:
			return visitorVisitVariableDefinition(visitor, node);
			break;
		
		case AST_FUNCTION_DEFINITION:
			return visitorVisitFunctionDefinition(visitor, node);
			break;
		
		case AST_VARIABLE:
			return visitorVisitVariable(visitor, node);
			break;
			
		case AST_FUNCTION_CALL:
			return visitorVisitFunctionCall(visitor, node);
			break;
			
		case AST_STRING:
			return visitorVisitString(visitor, node);
			break;
			
		case AST_NUMBER:
			return visitorVisitNumber(visitor, node);
			break;
			
		case AST_BOOLEAN:
			return visitorVisitBoolean(visitor, node);
			break;
			
		case AST_COMPOUND:
			return visitorVisitCompound(visitor, node);
			break;
			
		case AST_BINARY_OPERATION:
			return visitorVisitBinaryOperation(visitor, node);
			break;
			
		case AST_UNARY_OPERATION:
			return visitorVisitUnaryOperation(visitor, node);
			break;
			
		case AST_RETURN:
			return visitorVisitReturn(visitor, node);
			break;
		
		case AST_NOOP:
			return (void*)0;
			
	}
	
	printf("Uncaught statement of type '%d'\n", node->type);
	return (void*)0;
}

value_T* visitorVisitVariableDefinition(visitor_T* visitor, ast_T* node) {
	value_T* value = visitorVisit(visitor, node->variableDefinitionValue);
	
	scopeAddVariable(node->scope, node->variableDefinitionVariableName, value);
	
	//printf("Created variable '%s' with value '%s'\n", node->variableDefinitionVariableName, value->stringValue);
	
	return (void*)0;
}

value_T* visitorVisitFunctionDefinition(visitor_T* visitor, ast_T* node) {
	
	scopeAddFunctionDefinition(node->scope, node);
	
	return (void*)0;
}

value_T* visitorVisitVariable(visitor_T* visitor, ast_T* node) {
		
	variable_T* var = scopeGetVariable(node->scope, node->variableName);
				
	if (var != (void*)0) {
		return valueCopy(var->value);
	}
	
	printf("Undefined variable '%s'\n", node->variableName);
	exit(1);
}

value_T* visitorVisitFunctionCall(visitor_T* visitor, ast_T* node) {
	if (strcmp(node->functionCallName, "print") == 0) {
		return builtinFunctionPrint(
			visitor,
			node->functionCallArguments,
			node->functionCallArgumentsSize
		);
	}
	
	ast_T* funcDef = scopeGetFunctionDefinition(
		node->scope,
		node->functionCallName
	);

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
	
	// going through the function call arguments and adding the arguments to the functionbody's scope
	for (size_t i = 0; i < funcDef->functionDefinitionArgsSize; ++i) {
		ast_T* parameter = funcDef->functionDefinitionArgs[i];

		value_T* argument = visitorVisit(
			visitor,
			node->functionCallArguments[i]
		);

		scopeAddVariable(
			funcDef->functionDefinitionBody->scope,
			parameter->variableName,
			argument
		);
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

	scopeRemoveVariables(
		funcDef->functionDefinitionBody->scope,
		funcDef->functionDefinitionArgsSize
	);
	
	return result;
}

value_T* visitorVisitString(visitor_T* visitor, ast_T* node) {
	return valueInitString(node->stringValue);
}

value_T* visitorVisitNumber(visitor_T* visitor, ast_T* node) {
	return valueInitNumber(node->numberValue);
}

value_T* visitorVisitBoolean(visitor_T* visitor, ast_T* node) {
	return valueInitBoolean(node->booleanValue);
}

value_T* visitorVisitCompound(visitor_T* visitor, ast_T* node) {
	value_T* result = (void*)0;
	
	for (size_t i = 0; i < node->compoundSize; ++i) {
		
		if (visitor->shouldReturn) {
			break;	// if the visitor visited a return statement, it stops execution and returns
		}
		
		result = visitorVisit(visitor, node->compoundValue[i]);
	}
	
	// returns the value of its last statement, which will be useful for 
	// implementing return/expression handling
	return result;
}

value_T* visitorVisitBinaryOperation(visitor_T* visitor, ast_T* node) {
    value_T* left = visitorVisit(visitor, node->binaryOperationLeft);
    value_T* right = visitorVisit(visitor, node->binaryOperationRight);

    if (left == NULL || right == NULL) {
        valueFree(left);
        valueFree(right);
        return NULL;
    }

    if (left->type != VALUE_NUMBER || right->type != VALUE_NUMBER) {
        printf("Arithmetic operations require numeric operands\n");
        valueFree(left);
        valueFree(right);
        exit(1);
    }

    double result;

    switch (node->binaryOperationType) {
        case TOKEN_PLUS:
            result = left->numberValue + right->numberValue;
            break;

        case TOKEN_MINUS:
            result = left->numberValue - right->numberValue;
            break;

        case TOKEN_MULTIPLY:
            result = left->numberValue * right->numberValue;
            break;

        case TOKEN_DIVIDE:
            if (right->numberValue == 0) {
                printf("Division by zero\n");
                valueFree(left);
                valueFree(right);
                exit(1);
            }

            result = left->numberValue / right->numberValue;
            break;
            
        case TOKEN_EQUAL_EQUAL: {
			bool comparison = left->numberValue == right->numberValue;
			
			valueFree(left);
			valueFree(right);
        
			return valueInitBoolean(comparison);        
		}
			
		case TOKEN_LESS: {
			bool comparison = left->numberValue < right->numberValue;
			
			valueFree(left);
			valueFree(right);
        
			return valueInitBoolean(comparison);
		}
		
		case TOKEN_MORE: {
			bool comparison = left->numberValue > right->numberValue;
			
			valueFree(left);
			valueFree(right);
        
			return valueInitBoolean(comparison);
		}
		
		case TOKEN_LESS_THAN_EQUAL: {
			bool comparison = left->numberValue <= right->numberValue;
			
			valueFree(left);
			valueFree(right);
        
			return valueInitBoolean(comparison);
		}
		
		case TOKEN_MORE_THAN_EQUAL: {
			bool comparison = left->numberValue >= right->numberValue;
			
			valueFree(left);
			valueFree(right);
        
			return valueInitBoolean(comparison);
		}
		
		case TOKEN_NOT: {
			break;
		}
		
		case TOKEN_NOT_EQUAL: {
			bool comparison = left->numberValue != right->numberValue;
			
			valueFree(left);
			valueFree(right);
        
			return valueInitBoolean(comparison);
			break;
		}

        default:
            printf("Unknown arithmetic operator\n");
            valueFree(left);
            valueFree(right);
            exit(1);
    }

    valueFree(left);
    valueFree(right);

    return valueInitNumber(result);
}

value_T* visitorVisitUnaryOperation(visitor_T* visitor, ast_T* node) {
	value_T* operand = visitorVisit(visitor, node->unaryOperationOperand);
	
	if (operand == (void*)0) {
		return (void*)0;
	}
	
	switch(node->unaryOperationType) {
		case TOKEN_NOT: {
			if (operand->type != VALUE_BOOLEAN) {
				printf("Logical NOT requires a boolean operand\n");
				valueFree(operand);
				exit(1);
			}
			
			bool result = !operand->booleanValue;
			valueFree(operand);
			return valueInitBoolean(result);
		}
			
		default:
			printf("Unknown Unary Operator\n");
			valueFree(operand);
			exit(1);
	}
}

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


