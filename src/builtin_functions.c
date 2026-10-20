#include "include/builtin_functions.h"

#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>

/*
 * Evaluates and prints each argument, then frees its evaluated value.
 */
 
 value_T* builtinFunctionPrint(visitor_T* visitor, ast_T** args, int argsSize) {
	for (int i = 0; i < argsSize; ++i) {
		value_T* value = visitorVisit(visitor, args[i]);
		
		if (value == (void*)0) {
			continue;
		}
		
		switch (value->type) {
			
			case VALUE_NULL:
				printf("noval\n");
				break;
				
			case VALUE_INT:
				printf("%d\n", value->intValue);
				break;

			case VALUE_FLOAT:
				printf("%g\n", value->floatValue);
				break;
				
			case VALUE_BOOLEAN:
				printf("%s\n", value->booleanValue ? "true" : "false");
				break;
			
			case VALUE_STRING:
				printf("%s\n", value->stringValue);
				break;
				
			case VALUE_CONSTRUCT:
				printf("<construct>\n");
				break;
		}
		
		valueFree(value);
	}
	
	return (void*)0;
}

/*
 * Adds numeric arguments of the same type.
 */
value_T* builtinFunctionAdd(visitor_T* visitor, ast_T** args, int argsSize) {
    if (argsSize == 0) {
        return valueInitInt(0);
    }

    value_T* first = visitorVisit(visitor, args[0]);

    if (first == (void*)0) {
        return (void*)0;
    }

    if (first->type != VALUE_INT && first->type != VALUE_FLOAT) {
        printf("ERROR::BUILTIN::ADD: Expected numeric arguments\n");
        valueFree(first);
        exit(1);
    }

    int resultInt = 0;
    double resultFloat = 0.0;
    int resultType = first->type;

    if (resultType == VALUE_INT) {
        resultInt = first->intValue;
    } else {
        resultFloat = first->floatValue;
    }

    valueFree(first);

    for (int i = 1; i < argsSize; ++i) {
        value_T* value = visitorVisit(visitor, args[i]);

        if (value == NULL) {
            continue;
        }

        if (value->type != resultType) {
            printf("ERROR::BUILTIN::ADD: Arguments must have matching numeric types\n");
            valueFree(value);
            exit(1);
        }

        if (resultType == VALUE_INT) {
            resultInt += value->intValue;
        } else {
            resultFloat += value->floatValue;
        }

        valueFree(value);
    }

    if (resultType == VALUE_INT) {
        return valueInitInt(resultInt);
    }

    return valueInitFloat(resultFloat);
}

/*
 * Builtin raylib InitWindow Function
 */
value_T* builtinRaylibFuncInitWindow(visitor_T* visitor, ast_T** args, int argsSize) {
	if (argsSize == 0 || argsSize < 3) {
	 printf("Error::InitWindow, expected 3 arguments, received %d", argsSize);
	 exit(1);
	}

	// we expect a width, length, and title for the function call
	value_T* width = visitorVisit(visitor, args[0]);
	value_T* height = visitorVisit(visitor, args[1]);
	value_T* title = visitorVisit(visitor, args[2]);

	if (width->type != VALUE_FLOAT) {
		printf("Error::InitWindow, expected 'width' parameter to be of type float\n");
		exit(1);
	}
	 
	if (height->type != VALUE_FLOAT) {
		printf("Error::InitWindow, expected 'height' parameter to be of type float\n");
		exit(1);
	}
	
	if (title->type != VALUE_STRING) {
		printf("Error::InitWindow, expected, 'title' parameter to be of type string\n");
	}
	
	
}
