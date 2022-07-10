#pragma once

#include <QtCore/qglobal.h>

#if defined(COMPORT_LIBRARY)
#   define COMPORT_EXPORT Q_DECL_EXPORT
#else
#   define COMPORT_EXPORT Q_DECL_IMPORT
#endif

#define COMPORT_C_EXPORT extern "C" __declspec(dllexport)

