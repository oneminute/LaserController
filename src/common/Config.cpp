#include "Config.h"

#include <QDebug>
#include <QJsonDocument>

QMap<QString, Config::ConfigItem> Config::items;
Config Config::config;

Config::ConfigItem::ConfigItem(const QString & _prefix, const QString & _name, const QString & _description, const QVariant & _defaultValue, ConfigItemType _type)
    : prefix(_prefix)
    , name(_name)
    , description(_description)
    , value(_defaultValue)
    , defaultValue(_defaultValue)
    , type(_type)
    , modified(false)
{
    qDebug() << _prefix << _name;
    key = QString("%1/%2").arg(_prefix).arg(_name);
    if (!items.contains(key))
    {
        items.insert(key, *this);
    }
}

void Config::load()
{

}

void Config::save()
{

}

void Config::restore()
{
}

QString Config::path()
{
    return "config.json";
}
