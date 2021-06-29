﻿#include "Config.h"

#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

#include "ConfigItem.h"
#include "laser/LaserRegister.h"
#include "widget/InputWidgetWrapper.h"

QList<ConfigItemGroup*> Config::groups;
QMap<QString, ConfigItem*> Config::items;
//Config Config::config;

Config::Config()
{

}

void Config::load()
{
    groups.clear();
    items.clear();

    loadGeneralItems();
    loadLayersItems();
    loadUiItems();
    loadCuttingLayerItems();
    loadEngravingLayerItems();
    loadPathOptimizationItems();
    loadExportItems();
    loadDeviceItems();

    for (ConfigItemGroup* group : groups)
    {
        for (ConfigItem* item : group->items())
        {
            items.insert(item->fullName(), item);
        }
    }

        
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

        for (QJsonObject::ConstIterator i = group.constBegin(); i != group.constEnd(); i++)
        {
            QString name = i.key();
            QJsonObject itemObj = i.value().toObject();

            QString key = QString("%1/%2").arg(prefix).arg(name);
            //items[key].value = itemObj["value"].toVariant();
            //items[key].defaultValue = itemObj["defaultValue"].toVariant();

            //qDebug() << items[key].toString();
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
    for (QMap<QString, ConfigItem*>::ConstIterator i = items.constBegin(); i != items.constEnd(); i++)
    {
        QString key = i.key();
        const ConfigItem* item = i.value();

        /*if (!groups.contains(item.prefix))
        {
            groups.insert(item.prefix, QJsonObject());
        }
        QJsonObject& group = groups[item.prefix];

        QJsonObject itemObj;
        itemObj["name"] = item.name;
        itemObj["value"] = QJsonValue::fromVariant(item.value);
        itemObj["defaultValue"] = QJsonValue::fromVariant(item.defaultValue);
        itemObj["type"] = item.type;

        group[item.name] = itemObj;*/
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

bool Config::isModified()
{
    bool isModified = false;
    for (QMap<QString, ConfigItem*>::ConstIterator i = items.constBegin(); i != items.constEnd(); i++)
    {
        if (i.value()->isModified())
        {
            isModified = true;
            break;
        }
    }
    return isModified;
}

void Config::loadGeneralItems()
{
    ConfigItemGroup* group = new Config::General;
    groups.append(group);

    group->addConfigItem(
        "language"
        , tr("Language")
        , tr("Language for both UI and Business.")
        , 25
    );

    group->addConfigItem(
        "Unit"
        , tr("Unit")
        , tr("Global unit")
        , static_cast<int>(SU_MM)
    );
}

void Config::loadLayersItems()
{
    ConfigItemGroup* group = new Config::Layers;
    groups.append(group);

    group->addConfigItem(
        "maxLayersCount"
        , tr("Max Layers Count")
        , tr("Max Layers count.")
        , 16
    );
}

void Config::loadUiItems()
{
    ConfigItemGroup* group = new Config::Ui;
    groups.append(group);

    group->addConfigItem(
        "operationButtonIconSize"
        , tr("Operation Button Icon Size")
        , tr("Size of operation buttons' icons.")
        , 32
    );

    group->addConfigItem(
        "operationButtonWidth"
        , tr("Operation Button Width")
        , tr("Width of operation buttons.")
        , 60
    );

    group->addConfigItem(
        "operationButtonHeight",
        tr("Operation Button Height"),
        tr("Height of operation buttons."),
        60
    );

    group->addConfigItem(
        "operationButtonShowText",
        tr("Show Operation Button Text"),
        tr("Show text of operation button or not."),
        false
    );

    group->addConfigItem(
        "toolButtonSize",
        tr("Tool Button Size"),
        tr("Size of tool buttons."),
        32
    );

    group->addConfigItem(
        "colorButtonWidth",
        tr("Color Button Width"),
        tr("Width of color buttons."),
        30
    );

    group->addConfigItem(
        "colorButtonHeight",
        tr("Color Button Height"),
        tr("Height of color buttons."),
        30
    );
}

void Config::loadCuttingLayerItems()
{
    ConfigItemGroup* group = new Config::CuttingLayer;
    groups.append(group);

    group->addConfigItem(
        "minSpeed",
        tr("Min Speed"),
        tr("Min speed for cutting layers."),
        15
    );

    group->addConfigItem(
        "runSpeed",
        tr("Run Speed"),
        tr("Running speed for cutting layers."),
        60
    );

    group->addConfigItem(
        "laserPower",
        tr("Laser Power"),
        tr("Laser power for cutting layers."),
        80
    );

    group->addConfigItem(
        "minPowerRate",
        tr("Min Power Rate"),
        tr("The minimum power rate for cutting layers"),
        700
    );

    group->addConfigItem(
        "maxPowerRate",
        tr("Max Power Rate"),
        tr("The maximum power rate for cutting layers"),
        1000
    );
}

void Config::loadEngravingLayerItems()
{
    ConfigItemGroup* group = new Config::EngravingLayer;
    groups.append(group);

    group->addConfigItem(
        "minSpeed",
        tr("Min Speed"),
        tr("Min speed for engraving layers."),
        15
    );

    group->addConfigItem(
        "runSpeed",
        tr("Run Speed"),
        tr("Running speed for engraving layers."),
        60
    );

    group->addConfigItem(
        "laserPower",
        tr("Laser Power"),
        tr("Laser power for engraving layers."),
        80
    );

    group->addConfigItem(
        "minPowerRate",
        tr("Min Power Rate"),
        tr("The minimum power rate for engraving layers"),
        700
    );

    group->addConfigItem(
        "maxPowerRate",
        tr("Max Power Rate"),
        tr("The maximum power rate for engraving layers"),
        1000
    );

    group->addConfigItem(
        "useHalftone",
        tr("Use Halftone"),
        tr("Use halftone algorithm for bitmaps."),
        1000
    );

    group->addConfigItem(
        "LPI",
        tr("LPI"),
        tr("Lines per inch."),
        600
    );

    group->addConfigItem(
        "DPI",
        tr("DPI"),
        tr("Dots per inch."),
        600
    );
}

void Config::loadPathOptimizationItems()
{
    ConfigItemGroup* group = new Config::PathOptimization;
    groups.append(group);

    group->addConfigItem(
        "maxAnts",
        tr("Max ants"),
        tr("Max ants count."),
        100
    );

    group->addConfigItem(
        "maxIterations",
        tr("Max Iterations"),
        tr("Max iterations count."),
        500
    );

    group->addConfigItem(
        "maxTraverse",
        tr("Max Traverse"),
        tr("Max Traverse count."),
        2000
    );

    group->addConfigItem(
        "volatileRate",
        tr("Volatile Rate"),
        tr("Volatile of pheromones each iteration."),
        0.65
    );

    group->addConfigItem(
        "useGreedyAlgorithm",
        tr("Use Greedy Algorithm"),
        tr("Use greedy algorithm form path optimization."),
        true
    );

    group->addConfigItem(
        "maxStartingPoints",
        tr("Max Starting Points"),
        tr("Max starting points of each primitive."),
        8
    );

    group->addConfigItem(
        "maxStartingPointAnglesDiff",
        tr("Max Angles Diff"),
        tr("Max angles between starting points."),
        45
    );

}

void Config::loadExportItems()
{
    ConfigItemGroup* group = new Config::Export;
    groups.append(group);

    group->addConfigItem(
        "maxAnglesDiff",
        tr("Max Angles Diff"),
        tr("Max angles diff between tow points."),
        5.0
    );

    group->addConfigItem(
        "maxIntervalDistance",
        tr("Max Interval Distance"),
        tr("Max interval distance between tow points."),
        10.0
    );
}

void Config::loadDeviceItems()
{
    ConfigItemGroup* group = new Config::Device;
    groups.append(group);

    group->addConfigItem(
        "autoConnectFirst",
        tr("Auto Connect First"),
        tr("Auto connect to first com port when found multiple laser devices."),
        true
    );
}
