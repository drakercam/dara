#ifndef BUILTIN_FUNCTIONS_H
#define BUILTIN_FUNCTIONS_H

#include "visitor.h"
#include "ast.h"
#include "value.h"

/*
 * Prints the evaluated arguments of a Dara function call.
 */
value_T* builtinFunctionPrint(
    visitor_T* visitor,
    ast_T** args,
    int argsSize
);

/*
 * Adds numeric arguments of the same type.
 */
value_T* builtinFunctionAdd(
    visitor_T* visitor,
    ast_T** args,
    int argsSize
);

#endif
