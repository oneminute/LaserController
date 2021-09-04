#include "ConfigItemGroup.h"
#include "ConfigItem.h"
#include "Config.h"

#include <QJsonArray>

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
    QString description;
    QList<ConfigItem*> items;
    QMap<QString, ConfigItem*> itemsMap;
};

ConfigItemGroup::ConfigItemGroup(const QString& name, const QString& title, const QString& description, QObject* parent)
    : QObject(parent)
    , m_ptr(new ConfigItemGroupPrivate(this))
{
    Q_D(ConfigItemGroup);
    d->name = name;
    d->title = title;

    Config::groups.append(this);
    Config::groupsMap.insert(d->name, this);
}

ConfigItemGroup::~ConfigItemGroup()
{
    Q_D(ConfigItemGroup);
    qLogD << "ConfigItemGroup: " << this << " destroied";
    qDeleteAll(d->items);
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

QString ConfigItemGroup::description() const
{
    Q_D(const ConfigItemGroup);
    return d->description;
}

void ConfigItemGroup::addConfigItem(ConfigItem* item)
{
    Q_D(ConfigItemGroup);
    d->items.append(item);
    d->itemsMap.insert(item->name(), item);
}

ConfigItem* ConfigItemGroup::addConfigItem(const QString& name, const QString& title, 
    const QString& description, const QVariant& value, DataType dataType, bool advanced, 
    bool visible, StoreStrategy storeStrategy)
{
    ConfigItem* item = new ConfigItem(
        name
        , this
        , title
        , description
        , value
        , dataType
        , advanced
        , visible
        , storeStrategy
    );
    addConfigItem(item);
    return item;
}

QList<ConfigItem*>& ConfigItemGroup::items()
{
    Q_D(ConfigItemGroup);
    return d->items;
}

ConfigItem* ConfigItemGroup::configItem(const QString& name)
{
    Q_D(ConfigItemGroup);
    if (d->itemsMap.contains(name))
        return d->itemsMap[name];
    else
        return nullptr;
}

QJsonObject ConfigItemGroup::toJson() const
{
    Q_D(const ConfigItemGroup);
    QJsonObject group;
    for (ConfigItem* item : d->items)
    {
        group[item->name()] = item->toJson();
    }
    return group;
}

void ConfigItemGroup::fromJson(const QJsonObject& jsonObject)
{
    Q_D(ConfigItemGroup);
    for (QJsonObject::ConstIterator i = jsonObject.constBegin(); i != jsonObject.constEnd(); i++)
    {
        if (d->itemsMap.contains(i.key()))
        {
            ConfigItem* item = d->itemsMap[i.key()];
            item->fromJson(i.value().toObject());
            qLogD << *item;
        }
    }
}

bool ConfigItemGroup::isModified() const
{
    Q_D(const ConfigItemGroup);
    bool modified = false;
    for (ConfigItem* item : d->items)
    {
        if (item->isModified())
        {
            modified = true;
            break;
        }
    }
    return modified;
}

void ConfigItemGroup::confirm()
{
    Q_D(const ConfigItemGroup);
    for (ConfigItem* item : d->items)
    {
        item->confirm();
    }
}


