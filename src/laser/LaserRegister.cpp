#include "LaserRegister.h"

#include <QVariant>

#include "LaserDriver.h"

class LaserRegisterPrivate
{
    Q_DECLARE_PUBLIC(LaserRegister)
public:
    LaserRegisterPrivate(int addr, const QString& name, const QString& description, bool isSystem, bool readOnly, LaserRegister* ptr)
        : q_ptr(ptr)
        , address(addr)
        , name(name)
        , description(description)
        , isSystem(isSystem)
        , readOnly(readOnly)
    {

    }

    ~LaserRegisterPrivate()
    {
        qLogD << "register " << address << "(" << name << ") destroyed";
    }

    int address;
    QVariant value;
    QString name;
    QString description;
    bool readOnly;
    bool isSystem;
    LaserRegister* q_ptr;
};

LaserRegister::LaserRegister(int addr, const QString& name, const QString& description, bool isSystem, bool readOnly, LaserDriver* parent)
    : QObject(parent)
    , m_ptr(new LaserRegisterPrivate(addr, name, description, isSystem, readOnly, this))
{

}

LaserRegister::~LaserRegister()
{
}

int LaserRegister::address() const
{
    Q_D(const LaserRegister);
    return d->address;
}

QString LaserRegister::name() const
{
    Q_D(const LaserRegister);
    return d->name;
}

QString LaserRegister::description() const
{
    Q_D(const LaserRegister);
    return d->description;
}

bool LaserRegister::readOnly() const
{
    Q_D(const LaserRegister);
    return d->readOnly;
}

QVariant LaserRegister::value() const
{
    Q_D(const LaserRegister);
    return d->value;
}

void LaserRegister::setValue(const QVariant& value)
{
    Q_D(LaserRegister);
    d->value = value;
    emit valueChanged(value);
}

bool LaserRegister::readAsync()
{
    Q_D(LaserRegister);
    LaserDriver* driver = qobject_cast<LaserDriver*>(parent());
    QList<int> addresses;
    addresses << d->address;
    if (d->isSystem)
    {
        return driver->readSysParamFromCard(addresses);
    }
    else
    {
        return driver->readUserParamFromCard(addresses);
    }
}

bool LaserRegister::writeAsync()
{
    Q_D(LaserRegister);
    LaserDriver* driver = qobject_cast<LaserDriver*>(parent());
    LaserDriver::RegistersMap registerMap;
    registerMap.insert(d->address, d->value);
    if (d->isSystem)
    {
        return driver->writeSysParamToCard(registerMap);
    }
    else
    {
        return driver->writeUserParamToCard(registerMap);
    }
}
