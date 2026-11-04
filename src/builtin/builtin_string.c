#include "../include/builtin_registry.h"
#include "../include/builtin_functions.h"

void builtinRegisterString(scope_T* scope) {
	scopeRegisterNativeFunction(scope, "std:string.length", builtinFunctionPrint);
	scopeRegisterNativeFunction(scope, "std:string.concat", builtinFunctionPrint);
}
