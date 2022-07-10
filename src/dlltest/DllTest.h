#pragma once

#include "DllTestExport.h"

typedef void(__cdecl *CallbackFn)();

DLLTEST_C_EXPORT void __cdecl testFn1();

DLLTEST_C_EXPORT void __cdecl setCallback(CallbackFn fn);

DLLTEST_C_EXPORT void __cdecl doCallback();
