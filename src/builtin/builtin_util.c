#include "../include/builtin_registry.h"
#include "../include/builtin_functions.h"

static value_T* builtinFunctionTypeofAdapter(visitor_T* visitor, ast_T** args, int argsSize) {
	if (argsSize != 1) {
		visitorReportDiagnostic(
			visitor,
			argsSize > 0 ? args[0] : NULL,
			"E2033",
			"typeof expects exactly one argument."
		);

		return NULL;
	}

	return builtinFunctionTypeof(visitor, args[0]);
}

void builtinRegisterUtils(scope_T* scope) {
	scopeRegisterNativeFunction(scope, "std:util.typeof", builtinFunctionTypeofAdapter);
}
