#include "LaserRegister.h"

#include <QVariant>

#include "LaserApplication.h"
#include "LaserDevice.h"
//#include "LaserDriver.h"
#include "util/TypeUtils.h"

QMap<int, LaserRegister*> LaserRegister::userRegisters;
QMap<int, LaserRegister*> LaserRegister::systemRegisters;

class LaserRegisterPrivate
{
    Q_DECLARE_PUBLIC(LaserRegister)
public:
    LaserRegisterPrivate(int addr, const QString& name, DataType dataType, const QString& description, bool isSystem, bool readOnly, LaserRegister* ptr)
        : q_ptr(ptr)
        , address(addr)
        , name(name)
        , dataType(dataType)
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
    QString name;
    QString description;
    bool readOnly;
    bool isSystem;
    DataType dataType;
    StoreStrategy storeStrategy;
    LaserRegister* q_ptr;
};

LaserRegister::LaserRegister(int addr, const QString& name, DataType dataType, const QString& description, bool isSystem, bool readOnly, StoreStrategy storeStrategy, QObject* parent)
    : QObject(parent)
    , m_ptr(new LaserRegisterPrivate(addr, name, dataType, description, isSystem, readOnly, this))
{
    Q_D(LaserRegister);
    d->storeStrategy = storeStrategy;
    if (isSystem)
    {
        systemRegisters.insert(addr, this);
    }
    else
    {
        userRegisters.insert(addr, this);
    }
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

void LaserRegister::setValue(const QVariant& value)
{
    Q_D(LaserRegister);
    if (readOnly())
        return;

    if (storeStrategy() == SS_DIRECTLY)
    {
        write(value);
    }
}

DataType LaserRegister::dataType() const
{
    Q_D(const LaserRegister);
    return d->dataType;
}

void LaserRegister::setDataType(DataType dataType)
{
    Q_D(LaserRegister);
    d->dataType = dataType;
}

StoreStrategy LaserRegister::storeStrategy() const
{
    Q_D(const LaserRegister);
    return d->storeStrategy;
}

void LaserRegister::setStoreStrategy(StoreStrategy storeStrategy)
{
    Q_D(LaserRegister);
    d->storeStrategy = storeStrategy;
}

void LaserRegister::parse(const QString& raw)
{
    emit valueLoaded(typeUtils::stringToVariant(raw, dataType()));
}

bool LaserRegister::read()
{
    Q_D(LaserRegister);
    if (d->isSystem)
    {
        return LaserApplication::device->readSystemRegister(d->address);
    }
    else
    {
        return LaserApplication::device->readUserRegister(d->address);
    }
}

bool LaserRegister::write(const QVariant& value)
{
    Q_D(LaserRegister);
    if (d->isSystem)
    {
        return LaserApplication::device->writeSystemReigister(d->address, value);
    }
    else
    {
        return LaserApplication::device->writeUserReigister(d->address, value);
    }
}

QString LaserRegister::toString() const
{
    Q_D(const LaserRegister);
    return QString("%1").arg(d->address);
}

void LaserRegister::batchParse(const QString& raw, bool isSystem)
{
    QStringList segments = raw.split(";");
    for (QString seg : segments)
    {
        QStringList segItem = seg.split(",");
        if (segItem.length() != 2)
        {
            continue;
        }
        bool ok;
        int addr = segItem[0].toInt(&ok);
        if (!ok)
        {
            continue;
        }
        QString value = segItem[1];
        if (isSystem)
        {
            if (systemRegisters.contains(addr))
            {
                systemRegisters[addr]->parse(value);
            }
        }
        else
        {
            if (userRegisters.contains(addr))
            {
                userRegisters[addr]->parse(value);
            }
        }
    }

}
