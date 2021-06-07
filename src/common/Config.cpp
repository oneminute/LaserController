#include "Config.h"

#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

QMap<QString, Config::ConfigItem> Config::items;
Config Config::config;

Config::Config()
{

}

Config::ConfigItem::ConfigItem(const QString & _prefix, const QString & _name, const QVariant & _defaultValue, ConfigItemType _type, bool _advanced)
    : prefix(_prefix)
    , name(_name)
    , value(_defaultValue)
    , defaultValue(_defaultValue)
    , type(_type)
    , title("")
    , description("")
    , advanced(_advanced)
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
    GeneralLanguageItem()->title = QObject::tr("Language");
    GeneralLanguageItem()->description = QObject::tr("Language for both UI and Business.");
    GeneralUnitItem()->title = QObject::tr("Unit");
    GeneralUnitItem()->description = QObject::tr("Global unit.");
    LayersMaxLayersCountItem()->title = QObject::tr("Max layers count");
    LayersMaxLayersCountItem()->description = QObject::tr("Max layers count.");
    UIOperationButtonIconSizeItem()->title = QObject::tr("Operation Button Icon Size");
    UIOperationButtonIconSizeItem()->description = QObject::tr("Size of operation buttons' icons.");
    UIOperationButtonWidthItem()->title = QObject::tr("Operation Button Width");
    UIOperationButtonWidthItem()->description = QObject::tr("Width of operation buttons.");
    UIOperationButtonHeightItem()->title = QObject::tr("Operation Button Height");
    UIOperationButtonHeightItem()->description = QObject::tr("Height of operation buttons.");
    UIOperationButtonShowTextItem()->title = QObject::tr("Operation Button Height");
    UIOperationButtonShowTextItem()->description = QObject::tr("Show text of operation button or not.");
    UIToolButtonSizeItem()->title = QObject::tr("Tool Button Size");
    UIToolButtonSizeItem()->description = QObject::tr("Size of tool buttons.");
    UIColorButtonWidthItem()->title = QObject::tr("Color Button Width");
    UIColorButtonWidthItem()->description = QObject::tr("Width of color buttons.");
    UIColorButtonHeightItem()->title = QObject::tr("Color Button Height");
    UIColorButtonHeightItem()->description = QObject::tr("Height of color buttons.");
    CuttingLayerMinSpeedItem()->title = QObject::tr("Min Speed");
    CuttingLayerMinSpeedItem()->description = QObject::tr("Min speed for cutting layers.");
    CuttingLayerRunSpeedItem()->title = QObject::tr("Run Speed");
    CuttingLayerRunSpeedItem()->description = QObject::tr("Run speed for cutting layers.");
    CuttingLayerLaserPowerItem()->title = QObject::tr("Laser Power");
    CuttingLayerLaserPowerItem()->description = QObject::tr("Laser power for cutting layers.");
    CuttingLayerMinSpeedPowerItem()->title = QObject::tr("Min Speed Power");
    CuttingLayerMinSpeedPowerItem()->description = QObject::tr("Min speed power for cutting layers.");
    CuttingLayerRunSpeedPowerItem()->title = QObject::tr("Run Speed Power");
    CuttingLayerRunSpeedPowerItem()->description = QObject::tr("Run speed power for cutting layers.");
    EngravingLayerMinSpeedItem()->title = QObject::tr("Min Speed");
    EngravingLayerMinSpeedItem()->description = QObject::tr("Min speed for engraving layers.");
    EngravingLayerRunSpeedItem()->title = QObject::tr("Run Speed");
    EngravingLayerRunSpeedItem()->description = QObject::tr("Run speed for engraving layers.");
    EngravingLayerLaserPowerItem()->title = QObject::tr("Laser Power");
    EngravingLayerLaserPowerItem()->description = QObject::tr("Laser power for engraving layers.");
    EngravingLayerMinSpeedPowerItem()->title = QObject::tr("Min Speed Power");
    EngravingLayerMinSpeedPowerItem()->description = QObject::tr("Min speed power for engraving layers.");
    EngravingLayerRunSpeedPowerItem()->title = QObject::tr("Run Speed Power");
    EngravingLayerRunSpeedPowerItem()->description = QObject::tr("Run speed power for engraving layers.");
    EngravingLayerUseHalftoneItem()->title = QObject::tr("Use Halftone");
    EngravingLayerUseHalftoneItem()->description = QObject::tr("Use halftone algorithm for bitmaps.");
    EngravingLayerLPIItem()->title = QObject::tr("LPI");
    EngravingLayerLPIItem()->description = QObject::tr("Lines per inch.");
    EngravingLayerDPIItem()->title = QObject::tr("DPI");
    EngravingLayerDPIItem()->description = QObject::tr("Dots per inch.");
    OptimizePathMaxAntsItem()->title = QObject::tr("Max ants");
    OptimizePathMaxAntsItem()->description = QObject::tr("Max ants count.");
    OptimizePathMaxIterationsItem()->title = QObject::tr("Max iterations");
    OptimizePathMaxIterationsItem()->description = QObject::tr("Max iterations count.");
    OptimizePathMaxTraverseCountItem()->title = QObject::tr("Max Traverse");
    OptimizePathMaxTraverseCountItem()->description = QObject::tr("Max Traverse count.");
    OptimizePathVolatileRateItem()->title = QObject::tr("Volatile Rate");
    OptimizePathVolatileRateItem()->description = QObject::tr("Volatile of pheromones each iteration.");
    OptimizePathUseGreedyAlgorithmItem()->title = QObject::tr("Use Greedy algorithm");
    OptimizePathUseGreedyAlgorithmItem()->description = QObject::tr("Use greedy algorithm form path optimization.");
    OptimizePathMaxStartingPointsItem()->title = QObject::tr("Max starting points");
    OptimizePathMaxStartingPointsItem()->description = QObject::tr("Max starting points of each primitive.");
    OptimizePathMaxStartingPointAnglesDiffItem()->title = QObject::tr("Max Angles Diff");
    OptimizePathMaxStartingPointAnglesDiffItem()->description = QObject::tr("Max angles between starting points.");
    PltUtilsMaxAnglesDiffItem()->title = QObject::tr("Max angles diff");
    PltUtilsMaxAnglesDiffItem()->description = QObject::tr("Max angles diff between tow points.");
    PltUtilsMaxIntervalDistanceItem()->title = QObject::tr("Max interval distance");
    PltUtilsMaxIntervalDistanceItem()->description = QObject::tr("Max interval distance between tow points.");

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

bool Config::isModified()
{
    bool isModified = false;
    for (QMap<QString, ConfigItem>::ConstIterator i = items.constBegin(); i != items.constEnd(); i++)
    {
        if (i.value().modified)
        {
            isModified = true;
            break;
        }
    }
    return isModified;
}

