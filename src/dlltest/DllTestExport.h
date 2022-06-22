#pragma once

#include <QtCore/qglobal.h>

#if defined(DLLTEST_LIBRARY)
#   define DLLTEST_EXPORT Q_DECL_EXPORT
#else
#   define DLLTEST_EXPORT Q_DECL_IMPORT
#endif

#define DLLTEST_C_EXPORT extern "C" __declspec(dllexport)

