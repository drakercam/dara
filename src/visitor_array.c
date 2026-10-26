#include "include/visitor.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>

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

/*
 * Evaluates a method call belonging to the std:array namespace.
 */
value_T* visitorVisitArrayMethodCall(visitor_T* visitor, ast_T* node) {
	if (strcmp(node->methodCallName, "size") == 0 ||
		strcmp(node->methodCallName, "capacity") == 0) {
			
		if (node->methodCallArgumentsSize != 1) {
			visitorReportDiagnostic(visitor, node, "E2005", "Array size and capacity methods expect exactly one argument.");
			return (void*)0;
		}
		
		value_T* argument = visitorVisit(visitor, node->methodCallArguments[0]);
		
		if (!argument) {
			return (void*)0;
		}
		
		if (argument->type != VALUE_ARRAY || argument->arrayValue == (void*)0) {
			visitorReportDiagnostic(visitor, node, "E2006", "Array operations requires an array argument.");
			valueFree(argument);
			return (void*)0;
		}
		
		size_t resultValue;
		if (strcmp(node->methodCallName, "size") == 0) {
			resultValue = argument->arrayValue->size;
		}
		else if (strcmp(node->methodCallName, "capacity") == 0) {
			resultValue = argument->arrayValue->capacity;
		}
		else {
			valueFree(argument);
			return (void*)0;
		}
		
		valueFree(argument);
		
		if (resultValue > (size_t)INT_MAX) {
			visitorReportDiagnostic(visitor, node, "E2007", "Array result exceeds supported integer range.");
			return (void*)0;
		}
		
		value_T* result = valueInit(VALUE_INT);
		result->intValue = (int)resultValue;
		
		return result;
	}	
	
	visitorReportDiagnostic(visitor, node, "E2008", "Unknown std:array method");

	return (void*)0;
}
