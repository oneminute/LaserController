#include "ConfigItemGroup.h"
#include "ConfigItem.h"
#include "Config.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>

class ConfigItemGroupPrivate
{
    Q_DECLARE_PUBLIC(ConfigItemGroup)
public:
    ConfigItemGroupPrivate(ConfigItemGroup* ptr)
        : q_ptr(ptr)
        , lazy(false)
        , preSaveHook(nullptr)
    {}

    ConfigItemGroup* q_ptr;
    QString name;
    QString title;
    QString description;
    QList<ConfigItem*> items;
    QMap<QString, ConfigItem*> itemsMap;
    bool lazy;
    ConfigItemGroup::PreSaveHook preSaveHook;
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

ConfigItem* ConfigItemGroup::addConfigItem(const QString& name
    //, const QString& title
    //, const QString& description
    , const QVariant& value, DataType dataType, bool advanced, 
    bool visible, StoreStrategy storeStrategy)
{
    ConfigItem* item = new ConfigItem(
        name
        , this
        //, title
        //, description
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
        if (!item->exportable())
            continue;
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
            if (!item->exportable())
                continue;
            item->fromJson(i.value().toObject());
            qLogD << *item;
        }
    }
}

bool ConfigItemGroup::isLazy() const
{
    Q_D(const ConfigItemGroup);
    return d->lazy;
}

void ConfigItemGroup::setLazy(bool lazy)
{
    Q_D(ConfigItemGroup);
    d->lazy = lazy;
}

bool ConfigItemGroup::isModified() const
{
    Q_D(const ConfigItemGroup);
    bool modified = false;
    for (ConfigItem* item : d->items)
    {
        if (item->isModified() || item->isDirty())
        {
            modified = true;
            break;
        }
    }
    return modified;
}

bool ConfigItemGroup::needRelaunch() const
{
    Q_D(const ConfigItemGroup);
    bool needRelaunch = false;
    for (ConfigItem* item : d->items)
    {
        if (item->needRelaunch() && (item->isModified() || item->isDirty()))
        {
            needRelaunch = true;
            break;
        }
    }
    return needRelaunch;
}

void ConfigItemGroup::updateTitleAndDesc(const QString& title, const QString& desc)
{
    Q_D(ConfigItemGroup);
    d->title = title;
    d->description = desc;
}

bool ConfigItemGroup::load()
{
    QFile configFile(Config::configFilePath(name() + ".config"));
    if (configFile.exists())
    {
        if (!configFile.open(QFile::Text | QFile::ReadOnly))
        {
            configFile.close();
            return false;
        }

        QByteArray data = configFile.readAll();

        QJsonDocument doc(QJsonDocument::fromJson(data));

        QJsonObject json = doc.object();
        fromJson(json);
    }
    else
    {
        configFile.close();
        qLogD << "No valid config.json file found! We will create one.";
        save(true, true);
    }
    return true;
}

bool ConfigItemGroup::save(bool force, bool ignorePreSaveHook, bool ignoreApply)
{
    Q_D(ConfigItemGroup);
    if (!ignorePreSaveHook && d->preSaveHook)
    {
        if (!d->preSaveHook())
            return false;
    }

    if (!force && isLazy())
        return true;
    else
    {
        QFile configFile(Config::configFilePath(name() + ".config"));
        if (!configFile.open(QFile::Truncate | QFile::WriteOnly))
        {
            return false;
        }

        QJsonObject json = toJson();
        QJsonDocument doc(json);
        configFile.write(doc.toJson(QJsonDocument::JsonFormat::Indented));
        configFile.close();
        if (!ignoreApply)
        {
            for (ConfigItem* item : d->items)
            {
                item->apply();
                item->clearModified();
            }
        }
        return true;
    }
}

void ConfigItemGroup::setPreSaveHook(PreSaveHook hook)
{
    Q_D(ConfigItemGroup);
    d->preSaveHook = hook;
}


