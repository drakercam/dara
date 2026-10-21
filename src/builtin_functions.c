#include "include/builtin_functions.h"

#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include <string.h>

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
				
			case VALUE_COLOR:
				printf("<color.r=%d, color.g=%d, color.b=%d, color.a=%d>",
					value->colorValue.r, value->colorValue.g, value->colorValue.b, value->colorValue.a);
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
	if (argsSize != 3) {
	 printf("Runtime Error: rl:InitWindow expects 3 arguments, received %d\n", argsSize);
	 exit(EXIT_FAILURE);
	}

	// we expect a width, length, and title for the function call
	value_T* width = visitorVisit(visitor, args[0]);
	if (width == (void*)0) {
		return (void*)0;
	}
	value_T* height = visitorVisit(visitor, args[1]);
	if (height == (void*)0) {
		valueFree(width);
		return (void*)0;
	}
	value_T* title = visitorVisit(visitor, args[2]);
	if (title == (void*)0) {
		valueFree(width);
		valueFree(height);
		return (void*)0;
	}

	if (width->type != VALUE_INT || height->type != VALUE_INT || title->type != VALUE_STRING) {
		printf("Runtime Error: rl:InitWindow expects (int, int, string)\n");
		
		valueFree(width);
        valueFree(height);
        valueFree(title);
		
		exit(EXIT_FAILURE);
	}
	
	InitWindow(width->intValue, height->intValue, title->stringValue);
	valueFree(width);
	valueFree(height);
	valueFree(title);
	return valueInit(VALUE_NULL);
}

value_T* builtinRaylibFuncWindowShouldClose(visitor_T* visitor, ast_T** args, int argsSize) {
	if (argsSize != 0) {
		printf("Runtime Error: rl:WindowShouldClose expects 0 arguments\n");
		exit(EXIT_FAILURE);
	}
	
	value_T* result = valueInit(VALUE_BOOLEAN);
    result->booleanValue = WindowShouldClose();

    return result;
}

value_T* builtinRaylibFuncBeginDrawing(visitor_T* visitor, ast_T** args, int argsSize) {
	if (argsSize != 0) {
        printf("Runtime Error: rl:BeginDrawing expects 0 arguments\n");
        exit(1);
    }

    BeginDrawing();

    return valueInit(VALUE_NULL);
}

value_T* builtinRaylibFuncEndDrawing(visitor_T* visitor, ast_T** args, int argsSize) {
	if (argsSize != 0) {
        printf("Runtime Error: rl:EndDrawing expects 0 arguments\n");
        exit(1);
    }

    EndDrawing();

    return valueInit(VALUE_NULL);
}

value_T* builtinRaylibFuncCloseWindow(visitor_T* visitor, ast_T** args, int argsSize) {
	if (argsSize != 0) {
        printf("Runtime Error: rl:CloseWindow expects 0 arguments\n");
        exit(1);
    }

    CloseWindow();

    return valueInit(VALUE_NULL);
}

value_T* builtinRaylibFuncClearBackground(visitor_T* visitor, ast_T** args, int argsSize) {
	if (argsSize != 1) {
        printf(
            "Runtime Error: rl:ClearBackground expects 1 argument\n"
        );
        exit(EXIT_FAILURE);
    }

    value_T* color = visitorVisit(visitor, args[0]);

    if (color == (void*)0) {
        return (void*)0;
    }

    if (color->type != VALUE_COLOR) {
        printf(
            "Runtime Error: rl:ClearBackground expects a color argument\n"
        );
        valueFree(color);
        exit(EXIT_FAILURE);
    }

    ClearBackground((Color) {
        color->colorValue.r,
        color->colorValue.g,
        color->colorValue.b,
        color->colorValue.a
    });

    valueFree(color);

    return valueInit(VALUE_NULL);
}

value_T* builtinRaylibFuncDrawRectangle(visitor_T* visitor, ast_T** args, int argsSize) {
	if (argsSize != 5) {
        printf("Runtime Error: rl:DrawRectangle() expects 5 arguments.\n");
        exit(EXIT_FAILURE);
    }

    value_T* x = visitorVisit(visitor, args[0]);
    value_T* y = visitorVisit(visitor, args[1]);
    value_T* width = visitorVisit(visitor, args[2]);
    value_T* height = visitorVisit(visitor, args[3]);
    value_T* color = visitorVisit(visitor, args[4]);

    if (x->type != VALUE_INT ||
        y->type != VALUE_INT ||
        width->type != VALUE_INT ||
        height->type != VALUE_INT ||
        color->type != VALUE_COLOR) {

        printf(
            "Runtime Error: rl:DrawRectangle() expects "
            "(int, int, int, int, color).\n"
        );

        valueFree(x);
        valueFree(y);
        valueFree(width);
        valueFree(height);
        valueFree(color);

        exit(EXIT_FAILURE);
    }

    Color raylibColor = {
        color->colorValue.r,
        color->colorValue.g,
        color->colorValue.b,
        color->colorValue.a
    };

    DrawRectangle(
        x->intValue,
        y->intValue,
        width->intValue,
        height->intValue,
        raylibColor
    );

    valueFree(x);
    valueFree(y);
    valueFree(width);
    valueFree(height);
    valueFree(color);

    return valueInit(VALUE_NULL);
}

/*
 * Resolves supported raylib color constants.
 */
value_T* builtinRaylibGetConstant(const char* name) {
    if (strcmp(name, "rl:WHITE") == 0) {
        return valueInitColor(255, 255, 255, 255);
    }

    if (strcmp(name, "rl:BLUE") == 0) {
        return valueInitColor(0, 121, 241, 255);
    }

    if (strcmp(name, "rl:BLACK") == 0) {
        return valueInitColor(0, 0, 0, 255);
    }

    if (strcmp(name, "rl:RED") == 0) {
        return valueInitColor(230, 41, 55, 255);
    }

    if (strcmp(name, "rl:GREEN") == 0) {
        return valueInitColor(0, 228, 48, 255);
    }

    return (void*)0;
}
