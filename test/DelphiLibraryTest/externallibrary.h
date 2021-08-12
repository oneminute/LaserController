#ifndef EXTERNALLIBRARY_H
#define EXTERNALLIBRARY_H

#include <QObject>
#include <QLibrary>

class ExternalLibrary : public QObject
{
    Q_OBJECT
public:
    explicit ExternalLibrary(QObject *parent = nullptr);

    void addTwo();

    int showForm1();

    static void callback1(int a);

signals:

private:
    typedef int(__stdcall *FN_INT_INT_INT)(int a, int b);
    typedef void(__stdcall *FN_VOID_INT)(int);
    typedef int(__stdcall* FN_INT_VOID)();
    typedef void(__cdecl *FN_VOID_INT_CB)(int a);
    typedef void(__stdcall* FN_VOID_CALLBACK1)(FN_VOID_INT_CB);

    QLibrary m_library;

    FN_INT_INT_INT m_fnAddTwo;
    FN_VOID_INT m_fnInvCb1;
    FN_VOID_CALLBACK1 m_fnSetCallback1;
    FN_INT_VOID m_fnShowForm1;
};

#endif // EXTERNALLIBRARY_H
