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

#define CONFIG_ITEM_T(groupName, itemName, T) \
    static ConfigItem* itemName##Item() \
    { \
        return Config::groupsMap[#groupName]->configItem(#itemName); \
    } \
    static T itemName() \
    { \
        return itemName##Item()->value().value<T>(); \
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
    static void importFrom(const QString& filename);
    static void exportTo(const QString& filename);
    static void load();
    static void save(bool force, bool ignorePreSaveHook);
    static void restore();
    static QString configFilePath(const QString& filename = "Config.json");
    static bool isModified();
    static QList<ConfigItemGroup*> getGroups();
    static void updateTitlesAndDescriptions();
    static void destroy();

protected:
    static void loadGeneralItems();
    static void loadLayersItems();
    static void loadUiItems();
    static void loadCuttingLayerItems();
    static void loadEngravingLayerItems();
    static void loadFillingLayerItems();
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
        //CONFIG_ITEM(general, machiningUnit, qreal, toReal)

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
        CONFIG_ITEM(ui, visualGridSpacing, int, toInt)
        CONFIG_ITEM(ui, gridShapeDistance, qreal, toReal)
        CONFIG_ITEM(ui, objectShapeDistance, qreal, toReal)
        CONFIG_ITEM(ui, clickSelectionTolerance, qreal, toReal)
        CONFIG_ITEM(ui, validMaxRegion, int, toInt)
        CONFIG_ITEM(ui, splitterHandleWidth, int, toInt)
        CONFIG_ITEM(ui, autoRepeatDelay, int, toInt)
        CONFIG_ITEM(ui, showDocumentBoundingRect, bool, toBool)
        CONFIG_ITEM(ui, laserCursorTimeout, int, toInt)

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
        CONFIG_ITEM(cuttingLayer, runSpeed, int, toInt)
        CONFIG_ITEM(cuttingLayer, minPower, int, toInt)
        CONFIG_ITEM(cuttingLayer, maxPower, int, toInt)

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
        CONFIG_ITEM(engravingLayer, runSpeed, int, toInt)
        CONFIG_ITEM(engravingLayer, laserPower, qreal, toReal)
        CONFIG_ITEM(engravingLayer, minPower, qreal, toReal)
        CONFIG_ITEM(engravingLayer, maxPower, qreal, toReal)
        CONFIG_ITEM(engravingLayer, rowInterval, int, toInt)
        CONFIG_ITEM(engravingLayer, useHalftone, bool, toBool)
        CONFIG_ITEM(engravingLayer, halftoneAngles, qreal, toReal)
        CONFIG_ITEM(engravingLayer, halftoneGridSize, int, toInt)
        CONFIG_ITEM(engravingLayer, LPI, int, toInt)
        CONFIG_ITEM(engravingLayer, DPI, int, toInt)
        CONFIG_ITEM(engravingLayer, enableCutting, bool, toBool)

    private:
        friend class Config;
    };

    class FillingLayer : public ConfigItemGroup
    {
    protected:
        FillingLayer(QObject* parent = nullptr)
            : ConfigItemGroup("fillingLayer", tr("Filling Layer"), tr("Filling Layer"), parent)
        {}
        
    public:
        static ConfigItemGroup* group;
        CONFIG_ITEM(fillingLayer, runSpeed, int, toInt)
        CONFIG_ITEM(fillingLayer, minPower, qreal, toReal)
        CONFIG_ITEM(fillingLayer, maxPower, qreal, toReal)
        CONFIG_ITEM(fillingLayer, rowInterval, int, toInt)
        CONFIG_ITEM(fillingLayer, enableCutting, bool, toBool)
        CONFIG_ITEM(fillingLayer, fillingType, int, toInt)

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
        CONFIG_ITEM(pathOptimization, maxStartingPoints, int, toInt)
        CONFIG_ITEM(pathOptimization, groupingOrientation, int, toInt)
        CONFIG_ITEM(pathOptimization, groupingGridInterval, int, toInt)
        CONFIG_ITEM(pathOptimization, maxGroupSize, int, toInt)
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
        CONFIG_ITEM(export, enableSmallDiagonal, bool, toBool)
        CONFIG_ITEM(export, smallDiagonalLimitation, SmallDiagonalLimitation, value<SmallDiagonalLimitation>);
        CONFIG_ITEM(export, curveFlatteningThreshold, qreal, toReal)
        CONFIG_ITEM(export, gaussianFactorA, qreal, toReal)

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
        CONFIG_ITEM(device, autoConnectFirstCOM, bool, toBool)
        CONFIG_ITEM(device, startFrom, int, toInt)
        CONFIG_ITEM(device, jobOrigin, int, toInt)
        CONFIG_ITEM(device, xEnabled, bool, toBool)
        CONFIG_ITEM(device, yEnabled, bool, toBool)
        CONFIG_ITEM(device, zEnabled, bool, toBool)
        CONFIG_ITEM(device, userOrigin1, QPoint, toPoint)
        CONFIG_ITEM(device, userOrigin2, QPoint, toPoint)
        CONFIG_ITEM(device, userOrigin3, QPoint, toPoint)
        CONFIG_ITEM(device, userOriginSelected, int, toInt)
        CONFIG_ITEM(device, redLightOffset, QPointF, toPointF)
        //CONFIG_ITEM_T(device, userOrigins, QList<QPointF>, QList<QPointF>)

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
        CONFIG_ITEM(userRegister, scanLaserFrequency, qreal, toReal)
        CONFIG_ITEM(userRegister, scanReturnError, int, toInt)
        CONFIG_ITEM(userRegister, scanLaserPower, qreal, toReal)
        CONFIG_ITEM(userRegister, scanXResetEnabled, int, toInt)
        CONFIG_ITEM(userRegister, scanYResetEnabled, int, toInt)
        CONFIG_ITEM(userRegister, scanZResetEnabled, int, toInt)
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
        CONFIG_ITEM(userRegister, cuttingTurnOnDelay, qreal, toReal)
        CONFIG_ITEM(userRegister, cuttingTurnOffDelay, qreal, toReal)
        CONFIG_ITEM(userRegister, spotShotPower, qreal, toReal)

        CONFIG_ITEM(userRegister, fillingSpeed, qreal, toReal)
        CONFIG_ITEM(userRegister, fillingStartSpeed, qreal, toReal)
        CONFIG_ITEM(userRegister, fillingAcceleration, qreal, toReal)
        CONFIG_ITEM(userRegister, maxFillingPower, qreal, toReal)
        CONFIG_ITEM(userRegister, minFillingPower, qreal, toReal)
        CONFIG_ITEM(userRegister, fillingAccRatio, qreal, toReal)

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

        CONFIG_ITEM(systemRegister, yMaxLength, int, toInt)
        CONFIG_ITEM(systemRegister, yDirPhase, int, toInt)
        CONFIG_ITEM(systemRegister, yLimitPhase, int, toInt)
        CONFIG_ITEM(systemRegister, yZeroDev, int, toInt)
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
        CONFIG_ITEM(systemRegister, xPhaseEnabled, int, toInt)
        CONFIG_ITEM(systemRegister, yPhaseEnabled, int, toInt)
        CONFIG_ITEM(systemRegister, zPhaseEnabled, int, toInt)
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
    friend class ConfigDialog;
};

#endif // CONFIG_H
