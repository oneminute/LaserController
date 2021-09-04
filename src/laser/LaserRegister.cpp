#include "LaserRegister.h"

#include <QVariant>

#include "common/common.h"
#include "common/ConfigItem.h"
#include "LaserApplication.h"
#include "LaserDevice.h"
#include "util/TypeUtils.h"

class LaserRegisterPrivate
{
    Q_DECLARE_PUBLIC(LaserRegister)
public:
    LaserRegisterPrivate(LaserRegister* ptr)
        : q_ptr(ptr)
        , address(-1)
        , readOnly(false)
        , writeOnly(false)
        , isSystem(true)
        , configItem(nullptr)
    {

    }

    ~LaserRegisterPrivate()
    {
        qLogD << (isSystem ? "System" : "User") << " register " << address << "(" << address << ") destroyed";
    }

    LaserRegister* q_ptr;

    int address;
    bool readOnly;
    bool writeOnly;
    bool isSystem;
    QVariant value;
    ConfigItem* configItem;
};

LaserRegister::LaserRegister(int addr, ConfigItem* configItem, 
    bool isSystem, bool readOnly, bool writeOnly, QObject* parent)
    : QObject(parent)
    , d_ptr(new LaserRegisterPrivate(this))
{
    Q_ASSERT(configItem);

    Q_D(LaserRegister);
    d->address = addr;
    d->isSystem = isSystem;
    d->readOnly = readOnly;
    d->writeOnly = writeOnly;
    d->configItem = configItem;

    connect(configItem, &ConfigItem::valueChanged, this, &LaserRegister::setValue);
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
    return d->configItem->name();
}

QString LaserRegister::description() const
{
    Q_D(const LaserRegister);
    return d->configItem->description();
}

bool LaserRegister::readOnly() const
{
    Q_D(const LaserRegister);
    return d->readOnly;
}

bool LaserRegister::writeOnly() const
{
    Q_D(const LaserRegister);
    return d->writeOnly;
}

void LaserRegister::setValue(const QVariant& value, ModifiedBy modifiedBy)
{
    Q_D(LaserRegister);
    if (readOnly())
        return;

    if (modifiedBy == MB_Register ||
        modifiedBy == MB_RegisterConfirmed)
        return;

    d->value = value;

    if (storeStrategy() == SS_DIRECTLY)
    {
        write(value);
    }
}

DataType LaserRegister::dataType() const
{
    Q_D(const LaserRegister);
    return d->configItem->dataType();
}

StoreStrategy LaserRegister::storeStrategy() const
{
    Q_D(const LaserRegister);
    return d->configItem->storeType();
}

ConfigItem* LaserRegister::configItem() const
{
    Q_D(const LaserRegister);
    return d->configItem;
}

QVariant LaserRegister::value() const
{
    Q_D(const LaserRegister);
    return d->value;
}

void LaserRegister::parse(const QString& raw, ModifiedBy modifiedBy)
{
    Q_D(LaserRegister);
    d->value = typeUtils::stringToVariant(raw, dataType());
    d->configItem->setValue(d->value, modifiedBy);
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

