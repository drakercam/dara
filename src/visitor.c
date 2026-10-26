#include "include/visitor.h"
#include "include/scope.h"
#include "include/value.h"
#include "include/token.h"
#include "include/builtin_functions.h"
#include "include/diagnostic.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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
			
		case AST_CONSTRUCT_DEFINITION:
			return (void*)0;
			break;
			
		case AST_NAMESPACE_DEFINITION:
			return visitorVisitNamespaceDefinition(visitor, node);
			break;
			
		case AST_CONSTRUCT_INSTANTIATION:
			return visitorVisitConstructInstantiation(visitor, node);
			break;
		
		case AST_VARIABLE:
			return visitorVisitVariable(visitor, node);
			break;
			
		case AST_FUNCTION_CALL:
			return visitorVisitFunctionCall(visitor, node);
			break;
			
		case AST_ARRAY_CONSTRUCTOR:
			return visitorVisitArrayConstructor(visitor, node);
			break;
			
		case AST_METHOD_CALL:
			return visitorVisitMethodCall(visitor, node);
			
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
			
		case AST_IF_STATEMENT:
			return visitorVisitIfStatement(visitor, node);
			break;
			
		case AST_WHILE_LOOP:
			return visitorVisitWhileLoop(visitor, node);
			break;
			
		case AST_FOR_LOOP:
			return visitorVisitForLoop(visitor, node);
			break;
			
		case AST_ASSIGNMENT:
			return visitorVisitAssignment(visitor, node);
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
			
		case AST_MEMBER_ACCESS: {
			value_T* object = visitorVisit(
				visitor,
				node->memberAccessObject
			);

			if (object == NULL || object->type != VALUE_CONSTRUCT) {
				printf("Cannot access a field on a non-construct value\n");
				valueFree(object);
				exit(1);
			}

			value_T* field = valueConstructGetField(
				object,
				node->memberAccessField
			);

			if (field == NULL) {
				printf(
					"Construct has no field named '%s'\n",
					node->memberAccessField
				);
				valueFree(object);
				exit(1);
			}

			value_T* result = valueCopy(field);

			valueFree(object);

			return result;
		}
		
		case AST_NOOP:
			return (void*)0;
			
	}
	
	printf("Uncaught statement of type '%d'\n", node->type);
	return (void*)0;
}

bool visitorArgumentMatchesType(ast_T* parameter, value_T* argument) {
    char* type = parameter->variableDefinitionType;

    if (strcmp(type, "auto") == 0) {
        return true;
    }

    if (strcmp(type, "int") == 0) {
        return argument->type == VALUE_INT;
    }

    if (strcmp(type, "float") == 0) {
        return argument->type == VALUE_FLOAT;
    }

    if (strcmp(type, "string") == 0) {
        return argument->type == VALUE_STRING;
    }

    // A non-builtin type should match a construct's name.
    if (argument->type == VALUE_CONSTRUCT) {
        return strcmp(
            type,
            argument->constructValue.name
        ) == 0;
    }

    return false;
}

void visitorReportDiagnostic(visitor_T* visitor, ast_T* node, const char* code, const char* message) {
    char* ownedCode = calloc(strlen(code) + 1, sizeof(char));
    char* ownedMessage = calloc(strlen(message) + 1, sizeof(char));

    if (ownedCode == (void*)0 || ownedMessage == (void*)0) {
        free(ownedCode);
        free(ownedMessage);
        fprintf(stderr, "Failed to allocate diagnostic.\n");
        exit(EXIT_FAILURE);
    }

    strcpy(ownedCode, code);
    strcpy(ownedMessage, message);

    diagnostic_T* diagnostic = diagnosticInit(
        DIAGNOSTIC_RUNTIME,
        ownedCode,
        ownedMessage,
        node->location
    );

    if (diagnostic == (void*)0) {
        free(ownedCode);
        free(ownedMessage);
        fprintf(stderr, "Failed to allocate diagnostic.\n");
        exit(EXIT_FAILURE);
    }

    diagnosticPrint(diagnostic, visitor->fileName);
    diagnosticFree(diagnostic);
}

/*
 * Creates an array with a specified capacity and element type.
 */
value_T* visitorVisitArrayConstructor(visitor_T* visitor, ast_T* node) {
    size_t capacity = 1;

    // Evaluate the optional capacity expression.
    if (node->arrayConstructorCapacity != (void*)0) {
        value_T* capacityValue = visitorVisit(visitor, node->arrayConstructorCapacity);

        if (capacityValue == (void*)0 || capacityValue->type != VALUE_INT) {
            visitorReportDiagnostic(visitor, node, "E2002", "Array capacity must be an integer.");

            valueFree(capacityValue);
            return (void*)0;
        }

        if (capacityValue->intValue <= 0) {
            visitorReportDiagnostic(visitor, node, "E2002", "Array capacity must be greater than zero.");

            valueFree(capacityValue);
            return (void*)0;
        }

        capacity = (size_t)capacityValue->intValue;
        valueFree(capacityValue);
    }

    const char* elementTypeName = node->arrayConstructorElementType;
    int elementType;

    if (strcmp(elementTypeName, "auto") == 0) {
        // VALUE_NULL is the placeholder for an unconstrained element type.
        elementType = VALUE_NULL;
    } else if (strcmp(elementTypeName, "int") == 0) {
        elementType = VALUE_INT;
    } else if (strcmp(elementTypeName, "float") == 0) {
        elementType = VALUE_FLOAT;
    } else if (strcmp(elementTypeName, "string") == 0) {
        elementType = VALUE_STRING;
    } else if (strcmp(elementTypeName, "boolean") == 0) {
        elementType = VALUE_BOOLEAN;
    } else {
        visitorReportDiagnostic(visitor, node, "E2003", "Unsupported array element type.");

        return (void*)0;
    }

    array_T* array = calloc(1, sizeof(array_T));

    if (array == (void*)0) {
        visitorReportDiagnostic(visitor, node, "E2004","Failed to allocate array.");

        return (void*)0;
    }

    array->elements = calloc(capacity, sizeof(value_T*));

    if (array->elements == (void*)0) {
        free(array);

        visitorReportDiagnostic(visitor, node, "E2004", "Failed to allocate array elements.");

        return (void*)0;
    }

    array->elementTypeName = calloc(strlen(elementTypeName) + 1, sizeof(char));

    if (array->elementTypeName == (void*)0) {
        free(array->elements);
        free(array);

        visitorReportDiagnostic(visitor, node, "E2004", "Failed to allocate array element type.");

        return (void*)0;
    }

    strcpy(array->elementTypeName, elementTypeName);

    array->size = 0;
    array->capacity = capacity;
    array->elementType = elementType;

    value_T* result = valueInit(VALUE_ARRAY);

    if (result == (void*)0) {
        arrayFree(array);
        return (void*)0;
    }

    result->arrayValue = array;

    return result;
}
