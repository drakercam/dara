#ifndef BUILTIN_FUNCTIONS_H
#define BUILTIN_FUNCTIONS_H

#include "visitor.h"
#include "ast.h"
#include "value.h"

/*
 * Prints the evaluated arguments of a Dara function call.
 */
value_T* builtinFunctionPrint(visitor_T* visitor, ast_T** args, int argsSize);

/*
 * Adds numeric arguments of the same type.
 */
value_T* builtinFunctionAdd(visitor_T* visitor, ast_T** args, int argsSize);

/*
 * 	Checks the type of a variable at runtime and returns a string containing the type name
 */
 
value_T* builtinFunctionTypeof(visitor_T* visitor, ast_T* arg);

/*
 * Builtin raylib Functions
 */
value_T* builtinRaylibFuncInitWindow(visitor_T* visitor, ast_T** args, int argsSize);
value_T* builtinRaylibFuncWindowShouldClose(visitor_T* visitor, ast_T** args, int argsSize);
value_T* builtinRaylibFuncBeginDrawing(visitor_T* visitor, ast_T** args, int argsSize);
value_T* builtinRaylibFuncEndDrawing(visitor_T* visitor, ast_T** args, int argsSize);
value_T* builtinRaylibFuncCloseWindow(visitor_T* visitor, ast_T** args, int argsSize);
value_T* builtinRaylibFuncClearBackground(visitor_T* visitor, ast_T** args, int argsSize);

value_T* builtinRaylibFuncDrawRectangle(visitor_T* visitor, ast_T** args, int argsSize);

/*
 * Resolves a raylib constant such as rl:WHITE or rl:BLUE.
 * Returns NULL if the name is not a recognized raylib constant.
 */
value_T* builtinRaylibGetConstant(const char* name);

#endif
