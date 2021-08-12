#include "externallibrary.h"

#include <QDebug>

ExternalLibrary::ExternalLibrary(QObject *parent)
    : QObject(parent)
{

    m_library.setFileName("TestLib.dll");

    if (!m_library.load())
    {
        qDebug() << "Load library error.";
        return;
    }

    m_fnAddTwo = (FN_INT_INT_INT)m_library.resolve("AddTwo");
    m_fnInvCb1 = (FN_VOID_INT)m_library.resolve("InvCb1");
    m_fnSetCallback1 = (FN_VOID_CALLBACK1)m_library.resolve("SetCallback1");
    m_fnShowForm1 = (FN_INT_VOID)m_library.resolve("ShowForm1");

    m_fnSetCallback1(ExternalLibrary::callback1);
}

void ExternalLibrary::addTwo()
{
    int result = m_fnAddTwo(1, 2);
    qDebug() << "add two stdcall:" << result;
    m_fnInvCb1(result);
}

int ExternalLibrary::showForm1()
{
    return m_fnShowForm1();
}

void ExternalLibrary::callback1(int a)
{
    qDebug() << "a:" << a;
}
