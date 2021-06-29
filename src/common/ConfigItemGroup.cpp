#include "ConfigItemGroup.h"
#include "ConfigItem.h"

class ConfigItemGroupPrivate
{
    Q_DECLARE_PUBLIC(ConfigItemGroup)
public:
    ConfigItemGroupPrivate(ConfigItemGroup* ptr)
        : q_ptr(ptr)
    {}

    ConfigItemGroup* q_ptr;
    QString name;
    QString title;
    QList<ConfigItem*> items;
};

ConfigItemGroup::ConfigItemGroup(const QString& name, const QString& title, QObject* parent)
    : QObject(parent)
    , m_ptr(new ConfigItemGroupPrivate(this))
{
    Q_D(ConfigItemGroup);
    d->name = name;
    d->title = title;
}

ConfigItemGroup::~ConfigItemGroup()
{
}

QString ConfigItemGroup::name() const
{
    Q_D(const ConfigItemGroup);
    return d->name;
}

QString ConfigItemGroup::title() const
{
    Q_D(const ConfigItemGroup);
    return d->title;
}

void ConfigItemGroup::addConfigItem(ConfigItem* item)
{
    Q_D(ConfigItemGroup);
    d->items.append(item);
}

ConfigItem* ConfigItemGroup::addConfigItem(const QString& name, const QString& title, const QString& description, const QVariant& value, bool advanced, bool visible, StoreStrategy storeType)
{
    ConfigItem* item = new ConfigItem(
        name
        , this
        , title
        , description
        , value
        , advanced
        , visible
        , storeType
    );
    addConfigItem(item);
    return item;
}

QList<ConfigItem*>& ConfigItemGroup::items()
{
    Q_D(ConfigItemGroup);
    return d->items;
}
