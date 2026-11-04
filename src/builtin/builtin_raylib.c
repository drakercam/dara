#include "../include/builtin_registry.h"
#include "../include/builtin_functions.h"

void builtinRegisterRaylib(scope_T* scope) {
	scopeRegisterNativeFunction(scope, "rl:BeginDrawing", builtinRaylibFuncBeginDrawing);
	scopeRegisterNativeFunction(scope, "rl:ClearBackground", builtinRaylibFuncClearBackground);
	scopeRegisterNativeFunction(scope, "rl:CloseWindow", builtinRaylibFuncCloseWindow);
	scopeRegisterNativeFunction(scope, "rl:DrawRectangle", builtinRaylibFuncDrawRectangle);
	scopeRegisterNativeFunction(scope, "rl:EndDrawing", builtinRaylibFuncEndDrawing);
	scopeRegisterNativeFunction(scope, "rl:InitWindow", builtinRaylibFuncInitWindow);
	scopeRegisterNativeFunction(scope, "rl:WindowShouldClose", builtinRaylibFuncWindowShouldClose);
}
