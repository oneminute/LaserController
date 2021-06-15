#include "LaserRegister.h"

class LaserRegisterPrivate
{
    Q_DECLARE_PUBLIC(LaserRegister)
public:
    LaserRegisterPrivate(LaserRegister* ptr)
        : q_ptr(ptr)
    {

    }

    int address;
    LaserRegister* q_ptr;
};

LaserRegister::LaserRegister(QObject* parent)
    : QObject(parent)
{

}