#ifndef CONFIG_H
#define CONFIG_H

#include "common.h"
#include "ConfigItemGroup.h"
#include "ConfigItem.h"
#include "laser/LaserRegister.h"

#include <QObject>
#include <QString>
#include <QVariant>
#include <QMap>

#define CONFIG_ITEM(groupName, itemName, returnType, convertionMethod) \
    static ConfigItem* itemName##Item() \
    { \
        return Config::groupsMap[#groupName]->configItem(#itemName); \
    } \
    static returnType itemName() \
    { \
        return itemName##Item()->value().convertionMethod(); \
    }

class ConfigItem;
class InputWidgetWrapper;

class Config: public QObject
{
    Q_OBJECT
public:
    class General : public ConfigItemGroup
    {
    protected:
        General(QObject* parent = nullptr)
            : ConfigItemGroup("general", tr("General"), tr("General"), parent)
        {}

    public:
        CONFIG_ITEM(general, language, QString, toString)
        CONFIG_ITEM(general, unit, int, toInt)

    private:
        friend class Config;
    };

    class Layers : public ConfigItemGroup
    {
    protected:
        Layers(QObject* parent = nullptr)
            : ConfigItemGroup("layers", tr("Layers"), tr("Layers"), parent)
        {}

    public:
        CONFIG_ITEM(layers, maxLayersCount, int, toInt)

    private:
        friend class Config;
    };

    class Ui : public ConfigItemGroup
    {
    protected:
        Ui(QObject* parent = nullptr)
            : ConfigItemGroup("ui", tr("UI"), tr("UI"), parent)
        {}

    public:
        CONFIG_ITEM(ui, operationButtonIconSize, int, toInt)
        CONFIG_ITEM(ui, operationButtonWidth, int, toInt)
        CONFIG_ITEM(ui, operationButtonHeight, int, toInt)
        CONFIG_ITEM(ui, operationButtonShowText, bool, toBool)
        CONFIG_ITEM(ui, toolButtonSize, int, toInt)
        CONFIG_ITEM(ui, colorButtonWidth, int, toInt)
        CONFIG_ITEM(ui, colorButtonHeight, int, toInt)

    private:
        friend class Config;
    };

    class CuttingLayer : public ConfigItemGroup
    {
    protected:
        CuttingLayer(QObject* parent = nullptr)
            : ConfigItemGroup("cuttingLayer", tr("Cutting Layer"), tr("Cutting Layer"), parent)
        {}

    public:
        CONFIG_ITEM(cuttingLayer, minSpeed, int, toInt)
        CONFIG_ITEM(cuttingLayer, runSpeed, int, toInt)
        CONFIG_ITEM(cuttingLayer, laserPower, int, toInt)
        CONFIG_ITEM(cuttingLayer, minPowerRate, int, toInt)
        CONFIG_ITEM(cuttingLayer, maxPowerRate, int, toInt)

    private:
        friend class Config;
    };

    class EngravingLayer : public ConfigItemGroup
    {
    protected:
        EngravingLayer(QObject* parent = nullptr)
            : ConfigItemGroup("engravingLayer", tr("Engraving Layer"), tr("Engraving Layer"), parent)
        {}

    public:
        CONFIG_ITEM(engravingLayer, minSpeed, int, toInt)
        CONFIG_ITEM(engravingLayer, runSpeed, int, toInt)
        CONFIG_ITEM(engravingLayer, laserPower, int, toInt)
        CONFIG_ITEM(engravingLayer, minPowerRate, int, toInt)
        CONFIG_ITEM(engravingLayer, maxPowerRate, int, toInt)
        CONFIG_ITEM(engravingLayer, useHalftone, bool, toBool)
        CONFIG_ITEM(engravingLayer, LPI, int, toInt)
        CONFIG_ITEM(engravingLayer, DPI, int, toInt)

    private:
        friend class Config;
    };

    class PathOptimization : public ConfigItemGroup
    {
    protected:
        PathOptimization(QObject* parent = nullptr)
            : ConfigItemGroup("pathOptimization", tr("Path Optimization"), tr("Path Optimization"), parent)
        {}

    public:
        CONFIG_ITEM(pathOptimization, maxAnts, int, toInt)
        CONFIG_ITEM(pathOptimization, maxIterations, int, toInt)
        CONFIG_ITEM(pathOptimization, maxTraverse, int, toInt)
        CONFIG_ITEM(pathOptimization, volatileRate, qreal, toReal)
        CONFIG_ITEM(pathOptimization, useGreedyAlgorithm, bool, toBool)
        CONFIG_ITEM(pathOptimization, maxStartingPoints, int, toInt)
        CONFIG_ITEM(pathOptimization, startingPointAnglesDiff, qreal, toReal)

    private:
        friend class Config;
    };

    class Export : public ConfigItemGroup
    {
    protected:
        Export(QObject* parent = nullptr)
            : ConfigItemGroup("export", tr("Export"), tr("Export"), parent)
        {}

    public:
        CONFIG_ITEM(export, maxAnglesDiff, qreal, toReal)
        CONFIG_ITEM(export, maxIntervalDistance, int, toInt)

    private:
        friend class Config;
    };

    class Device : public ConfigItemGroup
    {
    protected:
        Device(QObject* parent = nullptr)
            : ConfigItemGroup("device", tr("Device"), tr("Device"), parent)
        {}

    public:
        CONFIG_ITEM(device, autoConnectFirst, bool, toBool)

    private:
        friend class Config;
    };

private:
    Config();

public:
    static void load();
    static void save();
    static void restore();
    static QString configFilePath();
    static bool isModified();
    static QList<ConfigItemGroup*> getGroups();

protected:
    static void loadGeneralItems();
    static void loadLayersItems();
    static void loadUiItems();
    static void loadCuttingLayerItems();
    static void loadEngravingLayerItems();
    static void loadPathOptimizationItems();
    static void loadExportItems();
    static void loadDeviceItems();

private:
    static QList<ConfigItemGroup*> groups;
    static QMap<QString, ConfigItemGroup*> groupsMap;

    friend class ConfigItemGroup;
};





#endif // CONFIG_H
