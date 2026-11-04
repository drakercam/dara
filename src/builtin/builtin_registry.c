#include "../include/builtin_registry.h"
#include "../include/builtin_functions.h"

void builtinRegisterAll(scope_T* scope) {
	/* Builtin registrations will be added here. */
	
	builtinRegisterIO(scope);
	builtinRegisterArray(scope);
	builtinRegisterRaylib(scope);
	builtinRegisterString(scope);
	builtinRegisterUtils(scope);
}
