#include "../include/builtin_registry.h"
#include "../include/builtin_functions.h"

void builtinRegisterIO(scope_T* scope) {
	scopeRegisterNativeFunction(scope, "std:io.print", builtinFunctionPrint);
}
