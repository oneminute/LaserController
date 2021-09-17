#ifndef CONFIG_H
#define CONFIG_H

#include "common.h"
#include "ConfigItemGroup.h"
#include "ConfigItem.h"
#include "laser/LaserRegister.h"
#include "scene/SmallDiagonalLimitation.h"

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

#define CONFIG_ITEM_T(groupName, itemName, returnType, T) \
    static ConfigItem* itemName##Item() \
    { \
        return Config::groupsMap[#groupName]->configItem(#itemName); \
    } \
    static returnType itemName() \
    { \
        return itemName##Item()->value<T>(); \
    }

class ConfigItem;
class InputWidgetWrapper;

class Config: public QObject
{
    Q_OBJECT

private:
    Config();
    ~Config();

public:
    static void init();
    static void load();
    static void save(const QString& mainCardId = "");
    static void restore();
    static QString configFilePath();
    static bool isModified();
    static QList<ConfigItemGroup*> getGroups();
    static void refreshTranslation();
    static void destroy();

protected:
    static void loadGeneralItems();
    static void loadLayersItems();
    static void loadUiItems();
    static void loadCuttingLayerItems();
    static void loadEngravingLayerItems();
    static void loadPathOptimizationItems();
    static void loadExportItems();
    static void loadDeviceItems();
    static void loadUserReigsters();
    static void loadSystemRegisters();
    static void loadDebug();

public:
    class General : public ConfigItemGroup
    {
    protected:
        General(QObject* parent = nullptr)
            : ConfigItemGroup("general", tr("General"), tr("General"), parent)
        {}

    public:
        static ConfigItemGroup* group;
        CONFIG_ITEM(general, language, int, toInt)
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
        static ConfigItemGroup* group;
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
        static ConfigItemGroup* group;
		CONFIG_ITEM(ui, operationButtonIconSize, int, toInt)
        CONFIG_ITEM(ui, operationButtonWidth, int, toInt)
        CONFIG_ITEM(ui, operationButtonHeight, int, toInt)
        CONFIG_ITEM(ui, operationButtonShowText, bool, toBool)
        CONFIG_ITEM(ui, toolButtonSize, int, toInt)
        CONFIG_ITEM(ui, colorButtonWidth, int, toInt)
        CONFIG_ITEM(ui, colorButtonHeight, int, toInt)
        CONFIG_ITEM(ui, gridContrast, int, toInt)
        CONFIG_ITEM(ui, visualGridSpacing, qreal, toDouble)
        CONFIG_ITEM(ui, gridShapeDistance, qreal, toDouble)
        CONFIG_ITEM(ui, objectShapeDistance, qreal, toDouble)
        CONFIG_ITEM(ui, clickSelectiontTolerance, qreal, toDouble)
        CONFIG_ITEM(ui, splitterHandleWidth, int, toInt)
        CONFIG_ITEM(ui, autoRepeatDelay, int, toInt)
        CONFIG_ITEM(ui, autoRepeatInterval, int, toInt)

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
        static ConfigItemGroup* group;
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
        static ConfigItemGroup* group;
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
        static ConfigItemGroup* group;
        CONFIG_ITEM(pathOptimization, maxAnts, int, toInt)
        CONFIG_ITEM(pathOptimization, maxIterations, int, toInt)
        CONFIG_ITEM(pathOptimization, maxTraverse, int, toInt)
        CONFIG_ITEM(pathOptimization, volatileRate, qreal, toReal)
        CONFIG_ITEM(pathOptimization, useGreedyAlgorithm, bool, toBool)
        CONFIG_ITEM(pathOptimization, maxStartingPoints, int, toInt)
        CONFIG_ITEM(pathOptimization, startingPointAnglesDiff, qreal, toReal)
        CONFIG_ITEM(pathOptimization, groupingOrientation, int, toInt)
        CONFIG_ITEM(pathOptimization, maxGroupingGridSize, qreal, toReal)
        CONFIG_ITEM(pathOptimization, searchingXYWeight, qreal, toReal)

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
        static ConfigItemGroup* group;
        CONFIG_ITEM(export, maxAnglesDiff, qreal, toReal)
        CONFIG_ITEM(export, maxIntervalDistance, int, toInt)
        CONFIG_ITEM(export, maxStartingPoints, int, toInt)
        CONFIG_ITEM(export, enableSmallDiagonal, bool, toBool)
        CONFIG_ITEM(export, smallDiagonalLimitation, SmallDiagonalLimitation, value<SmallDiagonalLimitation>);

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
        static ConfigItemGroup* group;
        CONFIG_ITEM(device, autoConnectFirst, bool, toBool)

        CONFIG_ITEM(device, startFrom, int, toInt)
        CONFIG_ITEM(device, jobOrigin, int, toInt)

        CONFIG_ITEM(device, xEnabled, bool, toBool)
        CONFIG_ITEM(device, yEnabled, bool, toBool)
        CONFIG_ITEM(device, zEnabled, bool, toBool)

    private:
        friend class Config;
    };

    class UserRegister : public ConfigItemGroup
    {
    protected:
        UserRegister(QObject* parent = nullptr)
            : ConfigItemGroup("userRegister", tr("User Registers"), tr("User registers"), parent)
        {}

    public:
        static ConfigItemGroup* group;
        CONFIG_ITEM(userRegister, head, QString, toString)
        CONFIG_ITEM(userRegister, accMode, int, toInt)
        CONFIG_ITEM(userRegister, cuttingMoveSpeed, int, toInt)
        CONFIG_ITEM(userRegister, cuttingMoveAcc, int, toInt)
        CONFIG_ITEM(userRegister, cuttingTurnSpeed, int, toInt)
        CONFIG_ITEM(userRegister, cuttingTurnAcc, int, toInt)
        CONFIG_ITEM(userRegister, cuttingWorkAcc, int, toInt)
        CONFIG_ITEM(userRegister, cuttingMoveSpeedFactor, int, toInt)
        CONFIG_ITEM(userRegister, cuttingWorkSpeedFactor, int, toInt)
        CONFIG_ITEM(userRegister, cuttingSpotSize, int, toInt)
        CONFIG_ITEM(userRegister, scanXStartSpeed, int, toInt)
        CONFIG_ITEM(userRegister, scanYStartSpeed, int, toInt)
        CONFIG_ITEM(userRegister, scanXAcc, int, toInt)
        CONFIG_ITEM(userRegister, scanYAcc, int, toInt)
        CONFIG_ITEM(userRegister, scanRowSpeed, int, toInt)
        CONFIG_ITEM(userRegister, scanRowInterval, qreal, toReal)
        CONFIG_ITEM(userRegister, scanReturnError, int, toInt)
        CONFIG_ITEM(userRegister, scanLaserPower, qreal, toReal)
        CONFIG_ITEM(userRegister, scanXResetEnabled, bool, toBool)
        CONFIG_ITEM(userRegister, scanYResetEnabled, bool, toBool)
        CONFIG_ITEM(userRegister, scanZResetEnabled, bool, toBool)
        CONFIG_ITEM(userRegister, resetSpeed, qreal, toReal)
        CONFIG_ITEM(userRegister, scanReturnPos, int, toInt)
        CONFIG_ITEM(userRegister, backlashXInterval, qreal, toReal)
        CONFIG_ITEM(userRegister, backlashYInterval, qreal, toReal)
        CONFIG_ITEM(userRegister, backlashZInterval, qreal, toReal)

        CONFIG_ITEM(userRegister, defaultRunSpeed, qreal, toReal)
        CONFIG_ITEM(userRegister, defaultMaxCuttingPower, qreal, toReal)
        CONFIG_ITEM(userRegister, defaultMinCuttingPower, qreal, toReal)
        CONFIG_ITEM(userRegister, defaultScanSpeed, qreal, toReal)
        CONFIG_ITEM(userRegister, maxScanGrayRatio, qreal, toReal)
        CONFIG_ITEM(userRegister, minScanGrayRatio, qreal, toReal)

    private:
        friend class Config;
    };

    class SystemRegister : public ConfigItemGroup
    {
    protected:
        SystemRegister(QObject* parent = nullptr)
            : ConfigItemGroup("systemRegister", tr("System Registers"), tr("System registers"), parent)
        {}

    public:
        static ConfigItemGroup* group;

        CONFIG_ITEM(systemRegister, head, int, toInt)
        CONFIG_ITEM(systemRegister, password, QString, toString)
        CONFIG_ITEM(systemRegister, storedPassword, QString, toString)
        CONFIG_ITEM(systemRegister, hardwareID1, QString, toString)
        CONFIG_ITEM(systemRegister, hardwareID2, QString, toString)
        CONFIG_ITEM(systemRegister, hardwareID3, QString, toString)
        CONFIG_ITEM(systemRegister, cdKey1, QString, toString)
        CONFIG_ITEM(systemRegister, cdKey2, QString, toString)
        CONFIG_ITEM(systemRegister, cdKey3, QString, toString)

        CONFIG_ITEM(systemRegister, sysRunTime, int, toInt)
        CONFIG_ITEM(systemRegister, laserRunTime, int, toInt)
        CONFIG_ITEM(systemRegister, sysRunNum, int, toInt)

        CONFIG_ITEM(systemRegister, xMaxLength, int, toInt)
        CONFIG_ITEM(systemRegister, xDirPhase, int, toInt)
        CONFIG_ITEM(systemRegister, xLimitPhase, int, toInt)
        CONFIG_ITEM(systemRegister, xZeroDev, int, toInt)
        CONFIG_ITEM(systemRegister, xStepLength, int, toInt)
        CONFIG_ITEM(systemRegister, xLimitNum, int, toInt)
        CONFIG_ITEM(systemRegister, xResetEnabled, int, toInt)
        CONFIG_ITEM(systemRegister, xMotorNum, int, toInt)
        CONFIG_ITEM(systemRegister, xMotorCurrent, int, toInt)
        CONFIG_ITEM(systemRegister, xStartSpeed, int, toInt)
        CONFIG_ITEM(systemRegister, xMaxSpeed, int, toInt)
        CONFIG_ITEM(systemRegister, xMaxAcceleration, int, toInt)
        CONFIG_ITEM(systemRegister, xUrgentAcceleration, int, toInt)

        CONFIG_ITEM(systemRegister, yMaxLength, qreal, toReal)
        CONFIG_ITEM(systemRegister, yDirPhase, bool, toBool)
        CONFIG_ITEM(systemRegister, yLimitPhase, bool, toBool)
        CONFIG_ITEM(systemRegister, yZeroDev, bool, toBool)
        CONFIG_ITEM(systemRegister, yStepLength, int, toInt)
        CONFIG_ITEM(systemRegister, yLimitNum, int, toInt)
        CONFIG_ITEM(systemRegister, yResetEnabled, int, toInt)
        CONFIG_ITEM(systemRegister, yMotorNum, int, toInt)
        CONFIG_ITEM(systemRegister, yMotorCurrent, int, toInt)
        CONFIG_ITEM(systemRegister, yStartSpeed, int, toInt)
        CONFIG_ITEM(systemRegister, yMaxSpeed, int, toInt)
        CONFIG_ITEM(systemRegister, yMaxAcceleration, int, toInt)
        CONFIG_ITEM(systemRegister, yUrgentAcceleration, int, toInt)

        CONFIG_ITEM(systemRegister, zMaxLength, int, toInt)
        CONFIG_ITEM(systemRegister, zDirPhase, int, toInt)
        CONFIG_ITEM(systemRegister, zLimitPhase, int, toInt)
        CONFIG_ITEM(systemRegister, zZeroDev, int, toInt)
        CONFIG_ITEM(systemRegister, zStepLength, int, toInt)
        CONFIG_ITEM(systemRegister, zLimitNum, int, toInt)
        CONFIG_ITEM(systemRegister, zResetEnabled, int, toInt)
        CONFIG_ITEM(systemRegister, zMotorNum, int, toInt)
        CONFIG_ITEM(systemRegister, zMotorCurrent, int, toInt)
        CONFIG_ITEM(systemRegister, zStartSpeed, int, toInt)
        CONFIG_ITEM(systemRegister, zMaxSpeed, int, toInt)
        CONFIG_ITEM(systemRegister, zMaxAcceleration, int, toInt)
        CONFIG_ITEM(systemRegister, zUrgentAcceleration, int, toInt)

        CONFIG_ITEM(systemRegister, laserMaxPower, int, toInt)
        CONFIG_ITEM(systemRegister, laserMinPower, int, toInt)
        CONFIG_ITEM(systemRegister, laserPowerFreq, int, toInt)
        CONFIG_ITEM(systemRegister, xPhaseEnabled, bool, toBool)
        CONFIG_ITEM(systemRegister, yPhaseEnabled, bool, toBool)
        CONFIG_ITEM(systemRegister, zPhaseEnabled, bool, toBool)
        CONFIG_ITEM(systemRegister, deviceOrigin, int, toInt)

    private:
        friend class Config;
    };

    class Debug : public ConfigItemGroup
    {
    protected:
        Debug(QObject* parent = nullptr)
            : ConfigItemGroup("debug", tr("Debug"), tr("Debug"), parent)
        {}

    public:
        static ConfigItemGroup* group;
        CONFIG_ITEM(debug, showPrimitiveName, bool, toBool)
        CONFIG_ITEM(debug, showPrimitiveFirstPoint, bool, toBool)
        CONFIG_ITEM(debug, generatePathImage, bool, toBool)
        CONFIG_ITEM(debug, generateMachiningImage, bool, toBool)
        CONFIG_ITEM(debug, enableOptimizeInteraction, bool, toBool)

    private:
        friend class Config;
    };

private:
    static QList<ConfigItemGroup*> groups;
    static QMap<QString, ConfigItemGroup*> groupsMap;

    friend class ConfigItemGroup;
};

#endif // CONFIG_H
