#ifndef BUILTIN_REGISTRY_H
#define BUILTIN_REGISTRY_H

#include "scope.h"

void builtinRegisterAll(scope_T* scope);

void builtinRegisterIO(scope_T* scope);
void builtinRegisterArray(scope_T* scope);
void builtinRegisterString(scope_T* scope);
void builtinRegisterRaylib(scope_T* scope);
void builtinRegisterUtils(scope_T* scope);

#endif
