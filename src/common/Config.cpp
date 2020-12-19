#include "Config.h"

#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

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
    QFile configFile(configFilePath());
    if (!configFile.open(QFile::Text | QFile::ReadOnly))
    {
        QMessageBox::warning(nullptr, QObject::tr("Open Failure"), QObject::tr("An error occured when opening configuration file!"));
        return;
    }

    QByteArray data = configFile.readAll();

    QJsonDocument doc(QJsonDocument::fromJson(data));

    QJsonObject json = doc.object();
    for (QJsonObject::ConstIterator g = json.constBegin(); g != json.constEnd(); g++)
    {
        QString prefix = g.key();
        QJsonObject group = g.value().toObject();

        //qDebug() << "group:" << prefix << group;
        for (QJsonObject::ConstIterator i = group.constBegin(); i != group.constEnd(); i++)
        {
            QString name = i.key();
            QJsonObject itemObj = i.value().toObject();

            QString key = QString("%1/%2").arg(prefix).arg(name);
            items[key].value = itemObj["value"].toVariant();
            items[key].defaultValue = itemObj["defaultValue"].toVariant();

            qDebug() << items[key].toString();
        }
    }

    configFile.close();
}

void Config::save()
{
    QFile configFile(configFilePath());
    if (!configFile.open(QFile::Truncate | QFile::WriteOnly))
    {
        QMessageBox::warning(nullptr, QObject::tr("Save Failure"), QObject::tr("An error occured when saving configuration file!"));
        return;
    }

    QJsonObject json;
    QMap<QString, QJsonObject> groups;
    for (QMap<QString, ConfigItem>::ConstIterator i = items.constBegin(); i != items.constEnd(); i++)
    {
        QString key = i.key();
        const ConfigItem& item = i.value();

        if (!groups.contains(item.prefix))
        {
            groups.insert(item.prefix, QJsonObject());
        }
        QJsonObject& group = groups[item.prefix];

        QJsonObject itemObj;
        itemObj["name"] = item.name;
        itemObj["value"] = QJsonValue::fromVariant(item.value);
        itemObj["defaultValue"] = QJsonValue::fromVariant(item.defaultValue);
        itemObj["type"] = item.type;

        group[item.name] = itemObj;
    }

    for (QMap<QString, QJsonObject>::ConstIterator i = groups.constBegin(); i != groups.constEnd(); i++)
    {
        json[i.key()] = i.value();
    }

    QJsonDocument doc(json);
    configFile.write(doc.toJson(QJsonDocument::JsonFormat::Indented));
    configFile.close();
}

void Config::restore()
{
}

QString Config::configFilePath()
{
    return "config.json";
}
