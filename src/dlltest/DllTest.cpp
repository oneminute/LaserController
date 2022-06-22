#include "DllTest.h"

#include <QDebug>

CallbackFn callbackFn = nullptr;

void __cdecl testFn1()
{
    qDebug() << "testFn1()";
}

void __cdecl setCallback(CallbackFn fn)
{
    qDebug() << "setCallback():" << fn;
    callbackFn = fn;
}

void __cdecl doCallback()
{
    qDebug() << "doCallback()";
    callbackFn();
}

