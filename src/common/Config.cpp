#include "Config.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

#include "ConfigItem.h"
#include "exception/LaserException.h"
#include "laser/LaserRegister.h"
#include "widget/InputWidgetWrapper.h"

QList<ConfigItemGroup*> Config::groups;
QMap<QString, ConfigItemGroup*> Config::groupsMap;

ConfigItemGroup* Config::General::group(nullptr);
ConfigItemGroup* Config::Layers::group(nullptr);
ConfigItemGroup* Config::Ui::group(nullptr);
ConfigItemGroup* Config::CuttingLayer::group(nullptr);
ConfigItemGroup* Config::EngravingLayer::group(nullptr);
ConfigItemGroup* Config::PathOptimization::group(nullptr);
ConfigItemGroup* Config::Export::group(nullptr);
ConfigItemGroup* Config::Device::group(nullptr);
ConfigItemGroup* Config::UserRegister::group(nullptr);
ConfigItemGroup* Config::SystemRegister::group(nullptr);

Config::Config()
{

}

void Config::load()
{
    qDeleteAll(groups);
    groups.clear();
    groupsMap.clear();

    loadGeneralItems();
    loadLayersItems();
    loadUiItems();
    loadCuttingLayerItems();
    loadEngravingLayerItems();
    loadPathOptimizationItems();
    loadExportItems();
    loadDeviceItems();
    loadUserReigsters();
    loadSystemRegisters();

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
        if (groupsMap.contains(g.key()))
        {
            ConfigItemGroup* group = groupsMap[g.key()];
            group->fromJson(g.value().toObject());
        }
    }

    configFile.close();
}

void Config::save(const QString& mainCardId)
{
    try {
        QFile configFile(configFilePath());
        if (!configFile.open(QFile::Truncate | QFile::WriteOnly))
        {
            QMessageBox::warning(nullptr, QObject::tr("Save Failure"), QObject::tr("An error occured when saving configuration file!"));
            return;
        }

        QJsonObject json;
        for (ConfigItemGroup* group : groups)
        {
            json[group->name()] = group->toJson();
        }
        QJsonDocument doc(json);
        configFile.write(doc.toJson(QJsonDocument::JsonFormat::Indented));
        configFile.close();

        if (!mainCardId.isEmpty() && !mainCardId.isNull())
        {
            QFile registerFile(QString("config/%1.lcr").arg(mainCardId));
            if (registerFile.open(QFile::Truncate | QFile::WriteOnly))
            {
                QJsonObject json;
                json[UserRegister::group->name()] = UserRegister::group->toJson();
                json[SystemRegister::group->name()] = SystemRegister::group->toJson();
                QJsonDocument doc(json);
                registerFile.write(doc.toJson(QJsonDocument::JsonFormat::Indented));
                registerFile.close();
            }
        }

        for (ConfigItemGroup* group : groups)
        {
            group->doModify();
        }
    }
    catch (...)
    {
        throw new LaserFileException(tr("Save config file error."));
    }
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
    for (ConfigItemGroup* group : groups)
    {
        if (group->isModified())
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
    Config::General::group = group;

    ConfigItem* language = group->addConfigItem(
        "language"
        , tr("Language")
        , tr("Language for both UI and Business.")
        , 25
    );
    language->setInputWidgetType(IWT_ComboBox);
    language->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (!comboBox)
                return;

            comboBox->addItem(QLocale::languageToString(QLocale::English), QLocale::English);
            comboBox->addItem(QLocale::languageToString(QLocale::Chinese), QLocale::Chinese);
            comboBox->setCurrentText(QLocale::languageToString(static_cast<QLocale::Language>(item->value().toInt())));
        }
    );

    ConfigItem* unit = group->addConfigItem(
        "unit"
        , tr("Unit")
        , tr("Global unit")
        , static_cast<int>(SU_MM)
    );
    unit->setInputWidgetType(IWT_ComboBox);
    unit->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (!comboBox)
                return;

            comboBox->addItem(tr("mm"), 4);
        }
    );
}

void Config::loadLayersItems()
{
    ConfigItemGroup* group = new Config::Layers;
    Config::Layers::group = group;

    ConfigItem* maxLayersCount = group->addConfigItem(
        "maxLayersCount"
        , tr("Max Layers Count")
        , tr("Max Layers count.")
        , 16
    );
    maxLayersCount->setInputWidgetType(IWT_EditSlider);
    maxLayersCount->setInputWidgetProperty("minimum", 8);
    maxLayersCount->setInputWidgetProperty("maximum", 40);
}

void Config::loadUiItems()
{
    ConfigItemGroup* group = new Config::Ui;
    Config::Ui::group = group;

    ConfigItem* operationButtonIconSize = group->addConfigItem(
        "operationButtonIconSize"
        , tr("Operation Button Icon Size")
        , tr("Size of operation buttons' icons.")
        , 32
    );
    operationButtonIconSize->setInputWidgetProperty("minimum", 16);
    operationButtonIconSize->setInputWidgetProperty("maximum", 64);

    ConfigItem* operationButtonWidth = group->addConfigItem(
        "operationButtonWidth"
        , tr("Operation Button Width")
        , tr("Width of operation buttons.")
        , 60
    );
    operationButtonWidth->setInputWidgetProperty("minimum", 32);
    operationButtonWidth->setInputWidgetProperty("maximum", 120);

    ConfigItem* operationButtonHeight = group->addConfigItem(
        "operationButtonHeight",
        tr("Operation Button Height"),
        tr("Height of operation buttons."),
        60
    );
    operationButtonHeight->setInputWidgetProperty("minimum", 32);
    operationButtonHeight->setInputWidgetProperty("maximum", 120);

    ConfigItem* operationButtonShowText = group->addConfigItem(
        "operationButtonShowText",
        tr("Show Operation Button Text"),
        tr("Show text of operation button or not."),
        false,
        DT_BOOL
    );
    operationButtonShowText->setInputWidgetType(IWT_CheckBox);

    ConfigItem* toolButtonSize = group->addConfigItem(
        "toolButtonSize",
        tr("Tool Button Size"),
        tr("Size of tool buttons."),
        32
    );
    toolButtonSize->setInputWidgetProperty("minimum", 16);
    toolButtonSize->setInputWidgetProperty("maximum", 64);

    ConfigItem* colorButtonWidth = group->addConfigItem(
        "colorButtonWidth",
        tr("Color Button Width"),
        tr("Width of color buttons."),
        30
    );
    colorButtonWidth->setInputWidgetProperty("minimum", 20);
    colorButtonWidth->setInputWidgetProperty("maximum", 60);

    ConfigItem* colorButtonHeight = group->addConfigItem(
        "colorButtonHeight",
        tr("Color Button Height"),
        tr("Height of color buttons."),
        30
    );
    colorButtonHeight->setInputWidgetProperty("minimum", 20);
    colorButtonHeight->setInputWidgetProperty("maximum", 60);
}

void Config::loadCuttingLayerItems()
{
    ConfigItemGroup* group = new Config::CuttingLayer;
    Config::CuttingLayer::group = group;

    ConfigItem* minSpeed = group->addConfigItem(
        "minSpeed",
        tr("Min Speed"),
        tr("Min speed for cutting layers."),
        15
    );
    minSpeed->setInputWidgetProperty("minimum", 1);
    minSpeed->setInputWidgetProperty("maximum", 1000);
    minSpeed->setInputWidgetProperty("textTemplate", "%1mm/s");
    minSpeed->setInputWidgetProperty("maximumLineEditWidth", 60);

    ConfigItem* runSpeed = group->addConfigItem(
        "runSpeed",
        tr("Run Speed"),
        tr("Running speed for cutting layers."),
        60
    );
    runSpeed->setInputWidgetProperty("minimum", 1);
    runSpeed->setInputWidgetProperty("maximum", 1000);
    runSpeed->setInputWidgetProperty("textTemplate", "%1mm/s");
    runSpeed->setInputWidgetProperty("maximumLineEditWidth", 60);

    ConfigItem* laserPower = group->addConfigItem(
        "laserPower",
        tr("Laser Power"),
        tr("Laser power for cutting layers."),
        8,
        DT_REAL
    );
    laserPower->setInputWidgetProperty("minimum", 0);
    laserPower->setInputWidgetProperty("maximum", 100);
    laserPower->setInputWidgetProperty("textTemplate", "%1%");

    ConfigItem* minPowerRate = group->addConfigItem(
        "minPowerRate",
        tr("Min Power Rate"),
        tr("The minimum power rate for cutting layers"),
        70,
        DT_REAL
    );
    minPowerRate->setInputWidgetProperty("minimum", 0);
    minPowerRate->setInputWidgetProperty("maximum", 100);
    minPowerRate->setInputWidgetProperty("textTemplate", "%1%");

    ConfigItem* maxPowerRate = group->addConfigItem(
        "maxPowerRate",
        tr("Max Power Rate"),
        tr("The maximum power rate for cutting layers"),
        100,
        DT_REAL
    );
    maxPowerRate->setInputWidgetProperty("minimum", 0);
    maxPowerRate->setInputWidgetProperty("maximum", 100);
    maxPowerRate->setInputWidgetProperty("textTemplate", "%1%");
}

void Config::loadEngravingLayerItems()
{
    ConfigItemGroup* group = new Config::EngravingLayer;
    Config::EngravingLayer::group = group;

    ConfigItem* minSpeed = group->addConfigItem(
        "minSpeed",
        tr("Min Speed"),
        tr("Min speed for engraving layers."),
        15
    );
    minSpeed->setInputWidgetProperty("minimum", 1);
    minSpeed->setInputWidgetProperty("maximum", 1000);
    minSpeed->setInputWidgetProperty("textTemplate", "%1mm/s");
    minSpeed->setInputWidgetProperty("maximumLineEditWidth", 60);

    ConfigItem* runSpeed = group->addConfigItem(
        "runSpeed",
        tr("Run Speed"),
        tr("Running speed for engraving layers."),
        60
    );
    runSpeed->setInputWidgetProperty("minimum", 1);
    runSpeed->setInputWidgetProperty("maximum", 1000);
    runSpeed->setInputWidgetProperty("textTemplate", "%1mm/s");
    runSpeed->setInputWidgetProperty("maximumLineEditWidth", 60);

    ConfigItem* laserPower = group->addConfigItem(
        "laserPower",
        tr("Laser Power"),
        tr("Laser power for engraving layers."),
        8
    );
    laserPower->setInputWidgetType(IWT_FloatEditSlider);
    laserPower->setInputWidgetProperty("minimum", 0);
    laserPower->setInputWidgetProperty("maximum", 100);
    laserPower->setInputWidgetProperty("textTemplate", "%1%");

    ConfigItem* minPowerRate = group->addConfigItem(
        "minPowerRate",
        tr("Min Power Rate"),
        tr("The minimum power rate for engraving layers"),
        70,
        DT_REAL
    );
    minPowerRate->setInputWidgetType(IWT_FloatEditSlider);
    minPowerRate->setInputWidgetProperty("minimum", 0);
    minPowerRate->setInputWidgetProperty("maximum", 100);
    minPowerRate->setInputWidgetProperty("textTemplate", "%1%");

    ConfigItem* maxPowerRate = group->addConfigItem(
        "maxPowerRate",
        tr("Max Power Rate"),
        tr("The maximum power rate for engraving layers"),
        100,
        DT_REAL
    );
    maxPowerRate->setInputWidgetProperty("minimum", 0);
    maxPowerRate->setInputWidgetProperty("maximum", 100);
    maxPowerRate->setInputWidgetProperty("textTemplate", "%1%");

    group->addConfigItem(
        "useHalftone",
        tr("Use Halftone"),
        tr("Use halftone algorithm for bitmaps."),
        true,
        DT_BOOL
    );

    ConfigItem* lpi = group->addConfigItem(
        "LPI",
        tr("LPI"),
        tr("Lines per inch."),
        600,
        DT_INT
    );
    lpi->setInputWidgetProperty("minimum", 1);
    lpi->setInputWidgetProperty("maximum", 1200);

    ConfigItem* dpi = group->addConfigItem(
        "DPI",
        tr("DPI"),
        tr("Dots per inch."),
        600,
        DT_INT
    );
    dpi->setInputWidgetProperty("minimum", 1);
    dpi->setInputWidgetProperty("maximum", 1200);
}

void Config::loadPathOptimizationItems()
{
    ConfigItemGroup* group = new Config::PathOptimization;
    Config::PathOptimization::group = group;

    ConfigItem* maxAnts = group->addConfigItem(
        "maxAnts",
        tr("Max ants"),
        tr("Max ants count."),
        100
    );
    maxAnts->setInputWidgetProperty("minimum", 1);
    maxAnts->setInputWidgetProperty("maximum", 100000);

    ConfigItem* maxIterations = group->addConfigItem(
        "maxIterations",
        tr("Max Iterations"),
        tr("Max iterations count."),
        500
    );
    maxIterations->setInputWidgetProperty("minimum", 1);
    maxIterations->setInputWidgetProperty("maximum", 2000);

    ConfigItem* maxTraverse = group->addConfigItem(
        "maxTraverse",
        tr("Max Traverse"),
        tr("Max Traverse count."),
        2000
    );
    maxTraverse->setInputWidgetProperty("minimum", 1);
    maxTraverse->setInputWidgetProperty("maximum", 200000000);

    ConfigItem* volatileRate = group->addConfigItem(
        "volatileRate",
        tr("Volatile Rate"),
        tr("Volatile of pheromones each iteration."),
        0.65,
        DT_REAL
    );
    volatileRate->setInputWidgetProperty("minimum", 0);
    volatileRate->setInputWidgetProperty("maximum", 1);
    volatileRate->setInputWidgetProperty("step", 0.01);
    volatileRate->setInputWidgetProperty("page", 0.1);
    volatileRate->setInputWidgetProperty("decimals", 2);

    group->addConfigItem(
        "useGreedyAlgorithm",
        tr("Use Greedy Algorithm"),
        tr("Use greedy algorithm form path optimization."),
        true,
        DT_BOOL
    );

    ConfigItem* maxStartingPoints = group->addConfigItem(
        "maxStartingPoints",
        tr("Max Starting Points"),
        tr("Max starting points of each primitive."),
        8
    );
    maxStartingPoints->setInputWidgetProperty("minimum", 1);
    maxStartingPoints->setInputWidgetProperty("maximum", 16);

    ConfigItem* maxStartingPointAnglesDiff = group->addConfigItem(
        "maxStartingPointAnglesDiff",
        tr("Max Angles Diff"),
        tr("Max angles between starting points."),
        45,
        DT_REAL
    );
    maxStartingPointAnglesDiff->setInputWidgetProperty("minimum", 1.0);
    maxStartingPointAnglesDiff->setInputWidgetProperty("maximum", 90.0);

}

void Config::loadExportItems()
{
    ConfigItemGroup* group = new Config::Export;
    Config::Export::group = group;

    ConfigItem* maxAnglesDiff = group->addConfigItem(
        "maxAnglesDiff",
        tr("Max Angles Diff"),
        tr("Max angles diff between tow points."),
        5.0,
        DT_REAL
    );
    maxAnglesDiff->setInputWidgetProperty("minimum", 1.0);
    maxAnglesDiff->setInputWidgetProperty("maximum", 90.0);

    ConfigItem* maxIntervalDistance = group->addConfigItem(
        "maxIntervalDistance",
        tr("Max Interval Distance"),
        tr("Max interval distance between tow points."),
        10
    );
    maxIntervalDistance->setInputWidgetProperty("minimum", 1);
    maxIntervalDistance->setInputWidgetProperty("maximum", 1000);
}

void Config::loadDeviceItems()
{
    ConfigItemGroup* group = new Config::Device;
    Config::Device::group = group;

    group->addConfigItem(
        "autoConnectFirst",
        tr("Auto Connect First"),
        tr("Auto connect to first com port when found multiple laser devices."),
        true,
        DT_BOOL
    );
}

void Config::loadUserReigsters()
{
    ConfigItemGroup* group = new Config::UserRegister;
    Config::UserRegister::group = group;

    ConfigItem* head = group->addConfigItem(
        "head",
        tr("Head Data"),
        tr("Head data for testing"),
        0x12345678,
        DT_INT
    );
    head->setReadOnly();
    head->setInputWidgetType(IWT_LineEdit);
    head->setInputWidgetProperty("readOnly", true);
    head->setLoadDataHook(
        [](const QVariant& value)
        {
            int intValue = value.toInt();
            QString hexValue = QString("0x%1").arg(intValue, 0, 16);
            return QVariant(hexValue);
        }
    );
    head->bindLaserRegister(0, false);

    ConfigItem* accMode = group->addConfigItem(
        "accMode",
        tr("Acceleration Mode"),
        tr("Acceleration mode"),
        0
    );
    accMode->setInputWidgetType(IWT_ComboBox);
    accMode->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (!comboBox)
                return;

            comboBox->addItem(tr("T"), 0);
            comboBox->addItem(tr("S1"), 1);
            comboBox->addItem(tr("S2"), 2);
            comboBox->addItem(tr("S3"), 3);
            comboBox->addItem(tr("S4"), 4);
            comboBox->addItem(tr("S5"), 5);
        }
    );
    accMode->bindLaserRegister(1, false);

    ConfigItem* cuttingMoveSpeed = group->addConfigItem(
        "cuttingMoveSpeed",
        tr("Cutting Move Speed"),
        tr("Cutting move speed"),
        15
    );
    cuttingMoveSpeed->setInputWidgetProperty("minimum", 1);
    cuttingMoveSpeed->setInputWidgetProperty("maximum", 1000);
    cuttingMoveSpeed->bindLaserRegister(2, false);

    ConfigItem* cuttingMoveAcc = group->addConfigItem(
        "cuttingMoveAcc",
        tr("Cutting Move Acceleration"),
        tr("Cutting Move Acceleration"),
        45
    );
    cuttingMoveAcc->setInputWidgetProperty("minimum", 1);
    cuttingMoveAcc->setInputWidgetProperty("maximum", 1000);
    cuttingMoveAcc->bindLaserRegister(3, false);

    ConfigItem* cuttingTurnSpeed = group->addConfigItem(
        "cuttingTurnSpeed",
        tr("Cutting Turn Speed"),
        tr("Cutting turn speed"),
        15
    );
    cuttingTurnSpeed->setInputWidgetProperty("minimum", 1);
    cuttingTurnSpeed->setInputWidgetProperty("maximum", 1000);
    cuttingTurnSpeed->bindLaserRegister(4, false);

    ConfigItem* cuttingTurnAcc = group->addConfigItem(
        "cuttingTurnAcc",
        tr("Cutting Turn Acceleration"),
        tr("Cutting turn acceleration"),
        45
    );
    cuttingTurnAcc->setInputWidgetProperty("minimum", 1);
    cuttingTurnAcc->setInputWidgetProperty("maximum", 1000);
    cuttingTurnAcc->bindLaserRegister(5, false);

    ConfigItem* cuttingWorkAcc = group->addConfigItem(
        "cuttingWorkAcc",
        tr("Cutting Work Acceleration"),
        tr("Cutting Work acceleration"),
        60
    );
    cuttingWorkAcc->setInputWidgetProperty("minimum", 1);
    cuttingWorkAcc->setInputWidgetProperty("maximum", 1000);
    cuttingWorkAcc->bindLaserRegister(6, false);

    ConfigItem* cuttingMoveSpeedFactor = group->addConfigItem(
        "cuttingMoveSpeedFactor",
        tr("Cutting Move Speed Factor"),
        tr("Cutting move speed factor"),
        2
    );
    cuttingMoveSpeedFactor->setInputWidgetProperty("minimum", 1);
    cuttingMoveSpeedFactor->setInputWidgetProperty("maximum", 100);
    cuttingMoveSpeedFactor->bindLaserRegister(7, false);

    ConfigItem* cuttingWorkSpeedFactor = group->addConfigItem(
        "cuttingWorkSpeedFactor",
        tr("Cutting Work Speed Factor"),
        tr("Cutting Work speed factor"),
        2
    );
    cuttingWorkSpeedFactor->setInputWidgetProperty("minimum", 1);
    cuttingWorkSpeedFactor->setInputWidgetProperty("maximum", 100);
    cuttingWorkSpeedFactor->bindLaserRegister(8, false);

    ConfigItem* cuttingSpotSize = group->addConfigItem(
        "cuttingSpotSize",
        tr("Cutting Spot Size"),
        tr("Cutting spot size"),
        30
    );
    cuttingSpotSize->setInputWidgetProperty("minimum", 1);
    cuttingSpotSize->setInputWidgetProperty("maximum", 100);
    cuttingSpotSize->bindLaserRegister(9, false);

    ConfigItem* scanXStartSpeed = group->addConfigItem(
        "scanXStartSpeed",
        tr("Scan X Start Speed"),
        tr("Scan x start speed"),
        15
    );
    scanXStartSpeed->setInputWidgetProperty("minimum", 1);
    scanXStartSpeed->setInputWidgetProperty("maximum", 1000);
    scanXStartSpeed->bindLaserRegister(10, false);

    ConfigItem* scanYStartSpeed = group->addConfigItem(
        "scanYStartSpeed",
        tr("Scan Y Start Speed"),
        tr("Scan y start speed"),
        15
    );
    scanYStartSpeed->setInputWidgetProperty("minimum", 1);
    scanYStartSpeed->setInputWidgetProperty("maximum", 1000);
    scanYStartSpeed->bindLaserRegister(11, false);

    ConfigItem* scanXAcc = group->addConfigItem(
        "scanXAcc",
        tr("Scan X Acceleration"),
        tr("Scan x acceleration"),
        5
    );
    scanXAcc->setInputWidgetProperty("minimum", 1);
    scanXAcc->setInputWidgetProperty("maximum", 1000);
    scanXAcc->bindLaserRegister(12, false);

    ConfigItem* scanYAcc = group->addConfigItem(
        "scanYAcc",
        tr("Scan Y Acceleration"),
        tr("Scan y acceleration"),
        45
    );
    scanYAcc->setInputWidgetProperty("minimum", 1);
    scanYAcc->setInputWidgetProperty("maximum", 1000);
    scanXAcc->bindLaserRegister(13, false);

    ConfigItem* scanRowSpeed = group->addConfigItem(
        "scanRowSpeed",
        tr("Scan Row Speed"),
        tr("Scan row speed"),
        15
    );
    scanRowSpeed->setInputWidgetProperty("minimum", 1);
    scanRowSpeed->setInputWidgetProperty("maximum", 1000);
    scanRowSpeed->bindLaserRegister(14, false);

    ConfigItem* scanRowInterval = group->addConfigItem(
        "scanRowInterval",
        tr("Scan Row Interval"),
        tr("Scan row interval"),
        7
    );
    scanRowInterval->setInputWidgetProperty("minimum", 1);
    scanRowInterval->setInputWidgetProperty("maximum", 100);
    scanRowInterval->bindLaserRegister(15, false);

    ConfigItem* scanReturnError = group->addConfigItem(
        "scanReturnError",
        tr("Scan Return Error"),
        tr("Scan return error"),
        0
    );
    scanReturnError->setInputWidgetProperty("minimum", 1);
    scanReturnError->setInputWidgetProperty("maximum", 100);
    scanReturnError->bindLaserRegister(16, false);

    ConfigItem* scanLaserPower = group->addConfigItem(
        "scanLaserPower",
        tr("Scan Laser Power"),
        tr("Scan laser power"),
        70,
        DT_REAL
    );
    scanLaserPower->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 10.0);
        }
    );
    scanLaserPower->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 10));
        }
    );
    scanLaserPower->setInputWidgetProperty("minimum", 1);
    scanLaserPower->setInputWidgetProperty("maximum", 100);
    scanLaserPower->bindLaserRegister(17, false);

    ConfigItem* scanXResetEnabled = group->addConfigItem(
        "scanXResetEnabled",
        tr("Scan X Reset Enabled"),
        tr("Scan x reset enabled"),
        true,
        DT_BOOL
    );
    scanXResetEnabled->bindLaserRegister(18, false);

    ConfigItem* scanYResetEnabled = group->addConfigItem(
        "scanYResetEnabled",
        tr("Scan Y Reset Enabled"),
        tr("Scan y reset enabled"),
        true,
        DT_BOOL
    );
    scanYResetEnabled->bindLaserRegister(19, false);

    ConfigItem* scanZResetEnabled = group->addConfigItem(
        "scanZResetEnabled",
        tr("Scan Z Reset Enabled"),
        tr("Scan z reset enabled"),
        true,
        DT_BOOL
    );
    scanZResetEnabled->bindLaserRegister(20, false);

    ConfigItem* scanReturnPos = group->addConfigItem(
        "scanReturnPos",
        tr("Scan Return pos"),
        tr("Scan return pos"),
        0
    );
    scanReturnPos->bindLaserRegister(21, false);

    ConfigItem* backlashXInterval = group->addConfigItem(
        "backlashXInterval",
        tr("Backlash X Interval"),
        tr("Backlash x interval"),
        0
    );
    backlashXInterval->setInputWidgetProperty("minimum", 1);
    backlashXInterval->setInputWidgetProperty("maximum", 100);
    backlashXInterval->bindLaserRegister(22, false);

    ConfigItem* backlashYInterval = group->addConfigItem(
        "backlashYInterval",
        tr("Backlash Y Interval"),
        tr("Backlash y interval"),
        0
    );
    backlashYInterval->setInputWidgetProperty("minimum", 1);
    backlashYInterval->setInputWidgetProperty("maximum", 100);
    backlashXInterval->bindLaserRegister(23, false);
}

void Config::loadSystemRegisters()
{
    ConfigItemGroup* group = new Config::SystemRegister;
    Config::SystemRegister::group = group;

    ConfigItem* sysRunTime = group->addConfigItem(
        "sysRunTime",
        tr("System run time"),
        tr("System run time"),
        0,
        DT_INT
    );
    sysRunTime->setReadOnly(true);
    sysRunTime->setInputWidgetType(IWT_LineEdit);
    sysRunTime->bindLaserRegister(9);

    ConfigItem* laserRunTime = group->addConfigItem(
        "laserRunTime",
        tr("Laser run time"),
        tr("Laser run time"),
        0,
        DT_INT
    );
    laserRunTime->setReadOnly(true);
    laserRunTime->setInputWidgetType(IWT_LineEdit);
    laserRunTime->bindLaserRegister(10);

    ConfigItem* sysRunNum = group->addConfigItem(
        "sysRunNum",
        tr("System run times"),
        tr("System run times"),
        0,
        DT_INT
    );
    sysRunNum->setReadOnly(true);
    sysRunNum->setInputWidgetType(IWT_LineEdit);
    sysRunNum->bindLaserRegister(11);

    ConfigItem* xMaxLength = group->addConfigItem(
        "xMaxLength",
        tr("X Max Length"),
        tr("X Max Length"),
        320,
        DT_INT
    );
    xMaxLength->setInputWidgetProperty("minimum", 1);
    xMaxLength->setInputWidgetProperty("maximum", 5000);
    xMaxLength->bindLaserRegister(12);

    ConfigItem* xDirPhase = group->addConfigItem(
        "xDirPhase",
        tr("X Max Length"),
        tr("X Max Length"),
        0,
        DT_INT
    );
    xDirPhase->setInputWidgetType(IWT_ComboBox);
    xDirPhase->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (!comboBox)
                return;

            comboBox->addItem(tr("0"), 0);
            comboBox->addItem(tr("1"), 1);
        }
    );
    xDirPhase->bindLaserRegister(13);

    ConfigItem* xLimitPhase = group->addConfigItem(
        "xLimitPhase",
        tr("X Limit Length"),
        tr("X Limit Length"),
        0
    );
    xLimitPhase->setInputWidgetType(IWT_ComboBox);
    xLimitPhase->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (!comboBox)
                return;

            comboBox->addItem(tr("0"), 0);
            comboBox->addItem(tr("1"), 1);
        }
    );
    xLimitPhase->bindLaserRegister(14);

    ConfigItem* xZeroDev = group->addConfigItem(
        "xZeroDev",
        tr("X Zero Dev"),
        tr("X Zero Dev"),
        2000
    );
    xZeroDev->setInputWidgetProperty("minimum", 1);
    xZeroDev->setInputWidgetProperty("maximum", 10000);
    xZeroDev->bindLaserRegister(15);

    ConfigItem* xStepLength = group->addConfigItem(
        "xStepLength",
        tr("X Step Length"),
        tr("X Step Length"),
        1
    );
    xStepLength->setInputWidgetProperty("minimum", 1);
    xStepLength->setInputWidgetProperty("maximum", 1000);
    xStepLength->bindLaserRegister(16);

    ConfigItem* xLimitNum = group->addConfigItem(
        "xLimitNum",
        tr("X Limit number"),
        tr("X Limit number"),
        0
    );
    xLimitNum->setInputWidgetType(IWT_ComboBox);
    xLimitNum->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (!comboBox)
                return;

            comboBox->addItem(tr("0"), 0);
            comboBox->addItem(tr("1"), 1);
            comboBox->addItem(tr("2"), 1);
            comboBox->addItem(tr("3"), 1);
        }
    );
    xLimitNum->bindLaserRegister(17);

    ConfigItem* xResetEnabled = group->addConfigItem(
        "xResetEnabled",
        tr("X Reset Enabled"),
        tr("X Reset Enabled"),
        true,
        DT_BOOL
    );
    xResetEnabled->bindLaserRegister(18);

    ConfigItem* xMotorNum = group->addConfigItem(
        "xMotorNum",
        tr("X Motor number"),
        tr("X Motor number"),
        0,
        DT_INT
    );
    xMotorNum->setInputWidgetType(IWT_ComboBox);
    xMotorNum->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (!comboBox)
                return;

            comboBox->addItem(tr("0"), 0);
            comboBox->addItem(tr("1"), 1);
            comboBox->addItem(tr("2"), 1);
            comboBox->addItem(tr("3"), 1);
        }
    );
    xMotorNum->bindLaserRegister(19);

    ConfigItem* xMotorCurrent = group->addConfigItem(
        "xMotorCurrent",
        tr("X Motor current"),
        tr("X Motor current"),
        50,
        DT_INT
    );
    xMotorCurrent->setInputWidgetProperty("minimum", 1);
    xMotorCurrent->setInputWidgetProperty("maximum", 100);
    xMotorCurrent->bindLaserRegister(20);

    ConfigItem* xStartSpeed = group->addConfigItem(
        "xStartSpeed",
        tr("X Start speed"),
        tr("X Start speed"),
        15,
        DT_REAL
    );
    xStartSpeed->setInputWidgetProperty("step", 0.001);
    xStartSpeed->setInputWidgetProperty("minimum", 1);
    xStartSpeed->setInputWidgetProperty("maximum", 1000);
    xStartSpeed->bindLaserRegister(21);

    ConfigItem* xMaxSpeed = group->addConfigItem(
        "xMaxSpeed",
        tr("X Max speed"),
        tr("X Max speed"),
        45,
        DT_REAL
    );
    xMaxSpeed->setInputWidgetProperty("step", 0.001);
    xMaxSpeed->setInputWidgetProperty("minimum", 1);
    xMaxSpeed->setInputWidgetProperty("maximum", 1000);
    xMaxSpeed->bindLaserRegister(22);

    ConfigItem* xMaxAcceleration = group->addConfigItem(
        "xMaxAcceleration",
        tr("X Max speed"),
        tr("X Max speed"),
        45,
        DT_REAL
    );
    xMaxAcceleration->setInputWidgetType(IWT_FloatEditSlider);
    xMaxAcceleration->setInputWidgetProperty("step", 0.001);
    xMaxAcceleration->setInputWidgetProperty("minimum", 1);
    xMaxAcceleration->setInputWidgetProperty("maximum", 1000);
    xMaxAcceleration->bindLaserRegister(23);

    ConfigItem* xUrgentAcceleration = group->addConfigItem(
        "xUrgentAcceleration",
        tr("X Urgent speed"),
        tr("X Urgent speed"),
        45,
        DT_REAL
    );
    xUrgentAcceleration->setInputWidgetType(IWT_FloatEditSlider);
    xUrgentAcceleration->setInputWidgetProperty("step", 0.001);
    xUrgentAcceleration->setInputWidgetProperty("minimum", 1);
    xUrgentAcceleration->setInputWidgetProperty("maximum", 1000);
    xUrgentAcceleration->bindLaserRegister(24);

    ConfigItem* yMaxLength = group->addConfigItem(
        "yMaxLength",
        tr("Y Max Length"),
        tr("Y Max Length"),
        210,
        DT_INT
    );
    yMaxLength->setInputWidgetProperty("minimum", 1);
    yMaxLength->setInputWidgetProperty("maximum", 5000);
    yMaxLength->bindLaserRegister(25);

    ConfigItem* yDirPhase = group->addConfigItem(
        "yDirPhase",
        tr("Y Dir Phase"),
        tr("Y Dir Phase"),
        0,
        DT_INT
    );
    yDirPhase->setInputWidgetType(IWT_ComboBox);
    yDirPhase->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (!comboBox)
                return;

            comboBox->addItem(tr("0"), 0);
            comboBox->addItem(tr("1"), 1);
        }
    );
    yDirPhase->bindLaserRegister(26);

    ConfigItem* yLimitPhase = group->addConfigItem(
        "yLimitPhase",
        tr("Y Limit Phase"),
        tr("Y Limit Phase"),
        0,
        DT_INT
    );
    yLimitPhase->setInputWidgetType(IWT_ComboBox);
    yLimitPhase->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (!comboBox)
                return;

            comboBox->addItem(tr("0"), 0);
            comboBox->addItem(tr("1"), 1);
        }
    );
    yLimitPhase->bindLaserRegister(27);

    ConfigItem* yZeroDev = group->addConfigItem(
        "yZeroDev",
        tr("Y Zero Dev"),
        tr("Y Zero Dev"),
        1000,
        DT_INT
    );
    yZeroDev->setInputWidgetProperty("minimum", 1);
    yZeroDev->setInputWidgetProperty("maximum", 10000);
    yZeroDev->bindLaserRegister(28);

    ConfigItem* yStepLength = group->addConfigItem(
        "yStepLength",
        tr("Y Step Length"),
        tr("Y Step Length"),
        1000,
        DT_INT
    );
    yStepLength->setInputWidgetProperty("minimum", 1);
    yStepLength->setInputWidgetProperty("maximum", 10000);
    yStepLength->bindLaserRegister(29);

    ConfigItem* yLimitNum = group->addConfigItem(
        "yLimitNum",
        tr("Y Limit number"),
        tr("Y Limit number"),
        0,
        DT_INT
    );
    yLimitNum->setInputWidgetType(IWT_ComboBox);
    yLimitNum->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (!comboBox)
                return;

            comboBox->addItem(tr("0"), 0);
            comboBox->addItem(tr("1"), 1);
            comboBox->addItem(tr("2"), 1);
            comboBox->addItem(tr("3"), 1);
        }
    );
    yLimitNum->bindLaserRegister(30);

    ConfigItem* yResetEnabled = group->addConfigItem(
        "yResetEnabled",
        tr("Y Reset Enabled"),
        tr("Y Reset Enabled"),
        true,
        DT_BOOL
    );
    yResetEnabled->bindLaserRegister(31);

    ConfigItem* yMotorNum = group->addConfigItem(
        "yMotorNum",
        tr("Y Motor number"),
        tr("Y Motor number"),
        0,
        DT_INT
    );
    yMotorNum->setInputWidgetType(IWT_ComboBox);
    yMotorNum->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (!comboBox)
                return;

            comboBox->addItem(tr("0"), 0);
            comboBox->addItem(tr("1"), 1);
            comboBox->addItem(tr("2"), 1);
            comboBox->addItem(tr("3"), 1);
        }
    );
    yMotorNum->bindLaserRegister(32);

    ConfigItem* yMotorCurrent = group->addConfigItem(
        "yMotorCurrent",
        tr("Y Motor current"),
        tr("Y Motor current"),
        50,
        DT_INT
    );
    yMotorCurrent->setInputWidgetProperty("minimum", 1);
    yMotorCurrent->setInputWidgetProperty("maximum", 100);
    yMotorCurrent->bindLaserRegister(33);

    ConfigItem* yStartSpeed = group->addConfigItem(
        "yStartSpeed",
        tr("Y Start speed"),
        tr("Y Start speed"),
        15,
        DT_INT
    );
    yStartSpeed->setInputWidgetProperty("minimum", 1);
    yStartSpeed->setInputWidgetProperty("maximum", 100);
    yStartSpeed->bindLaserRegister(34);

    ConfigItem* yMaxSpeed = group->addConfigItem(
        "yMaxSpeed",
        tr("Y Max speed"),
        tr("Y Max speed"),
        45,
        DT_REAL
    );
    yMaxSpeed->setInputWidgetType(IWT_FloatEditSlider);
    yMaxSpeed->setInputWidgetProperty("step", 0.001);
    yMaxSpeed->setInputWidgetProperty("minimum", 1);
    yMaxSpeed->setInputWidgetProperty("maximum", 1000);
    yMaxSpeed->bindLaserRegister(35);

    ConfigItem* yMaxAcceleration = group->addConfigItem(
        "yMaxAcceleration",
        tr("Y Max speed"),
        tr("Y Max speed"),
        45,
        DT_REAL
    );
    yMaxAcceleration->setInputWidgetType(IWT_FloatEditSlider);
    yMaxAcceleration->setInputWidgetProperty("step", 0.001);
    yMaxAcceleration->setInputWidgetProperty("minimum", 1);
    yMaxAcceleration->setInputWidgetProperty("maximum", 1000);
    yMaxAcceleration->bindLaserRegister(36);

    ConfigItem* yUrgentAcceleration = group->addConfigItem(
        "yUrgentAcceleration",
        tr("Y Urgent speed"),
        tr("Y Urgent speed"),
        20,
        DT_REAL
    );
    yUrgentAcceleration->setInputWidgetType(IWT_FloatEditSlider);
    yUrgentAcceleration->setInputWidgetProperty("step", 0.001);
    yUrgentAcceleration->setInputWidgetProperty("minimum", 1);
    yUrgentAcceleration->setInputWidgetProperty("maximum", 1000);
    yUrgentAcceleration->bindLaserRegister(37);

    ConfigItem* zMaxLength = group->addConfigItem(
        "zMaxLength",
        tr("Z Max Length"),
        tr("Z Max Length"),
        200,
        DT_INT
    );
    zMaxLength->setInputWidgetProperty("minimum", 1);
    zMaxLength->setInputWidgetProperty("maximum", 5000);
    zMaxLength->bindLaserRegister(38);

    ConfigItem* zDirPhase = group->addConfigItem(
        "zDirPhase",
        tr("Z Dir Phase"),
        tr("Z Dir Phase"),
        0,
        DT_INT
    );
    zDirPhase->setInputWidgetType(IWT_ComboBox);
    zDirPhase->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (!comboBox)
                return;

            comboBox->addItem(tr("0"), 0);
            comboBox->addItem(tr("1"), 1);
        }
    );
    zDirPhase->bindLaserRegister(39);

    ConfigItem* zLimitPhase = group->addConfigItem(
        "zLimitPhase",
        tr("Z Limit Length"),
        tr("Z Limit Length"),
        0,
        DT_INT
    );
    zLimitPhase->setInputWidgetType(IWT_ComboBox);
    zLimitPhase->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (!comboBox)
                return;

            comboBox->addItem(tr("0"), 0);
            comboBox->addItem(tr("1"), 1);
        }
    );
    zLimitPhase->bindLaserRegister(40);

    ConfigItem* zZeroDev = group->addConfigItem(
        "zZeroDev",
        tr("Z Zero Dev"),
        tr("Z Zero Dev"),
        2000,
        DT_INT
    );
    zZeroDev->setInputWidgetProperty("minimum", 1);
    zZeroDev->setInputWidgetProperty("maximum", 10000);
    zZeroDev->bindLaserRegister(41);

    ConfigItem* zStepLength = group->addConfigItem(
        "zStepLength",
        tr("Z Step Length"),
        tr("Z Step Length"),
        20,
        DT_INT
    );
    zStepLength->setInputWidgetProperty("minimum", 1);
    zStepLength->setInputWidgetProperty("maximum", 10000);
    zStepLength->bindLaserRegister(42);

    ConfigItem* zLimitNum = group->addConfigItem(
        "zLimitNum",
        tr("Z Limit number"),
        tr("Z Limit number"),
        0,
        DT_INT
    );
    zLimitNum->setInputWidgetType(IWT_ComboBox);
    zLimitNum->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (!comboBox)
                return;

            comboBox->addItem(tr("0"), 0);
            comboBox->addItem(tr("1"), 1);
            comboBox->addItem(tr("2"), 1);
            comboBox->addItem(tr("3"), 1);
        }
    );
    zLimitNum->bindLaserRegister(43);

    ConfigItem* zResetEnabled = group->addConfigItem(
        "zResetEnabled",
        tr("Z Reset Enabled"),
        tr("Z Reset Enabled"),
        true,
        DT_BOOL
    );
    zResetEnabled->bindLaserRegister(44);

    ConfigItem* zMotorNum = group->addConfigItem(
        "zMotorNum",
        tr("Z Motor number"),
        tr("Z Motor number"),
        0,
        DT_INT
    );
    zMotorNum->setInputWidgetType(IWT_ComboBox);
    zMotorNum->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (!comboBox)
                return;

            comboBox->addItem(tr("0"), 0);
            comboBox->addItem(tr("1"), 1);
            comboBox->addItem(tr("2"), 1);
            comboBox->addItem(tr("3"), 1);
        }
    );
    zMotorNum->bindLaserRegister(45);

    ConfigItem* zMotorCurrent = group->addConfigItem(
        "zMotorCurrent",
        tr("Z Motor current"),
        tr("Z Motor current"),
        50,
        DT_INT
    );
    zMotorCurrent->setInputWidgetProperty("minimum", 1);
    zMotorCurrent->setInputWidgetProperty("maximum", 100);
    zMotorCurrent->bindLaserRegister(46);

    ConfigItem* zStartSpeed = group->addConfigItem(
        "zStartSpeed",
        tr("Z Start speed"),
        tr("Z Start speed"),
        15,
        DT_INT
    );
    zStartSpeed->setInputWidgetProperty("minimum", 1);
    zStartSpeed->setInputWidgetProperty("maximum", 100);
    zStartSpeed->bindLaserRegister(47);

    ConfigItem* zMaxSpeed = group->addConfigItem(
        "zMaxSpeed",
        tr("Z Max speed"),
        tr("Z Max speed"),
        10,
        DT_REAL
    );
    zMaxSpeed->setInputWidgetType(IWT_FloatEditSlider);
    zMaxSpeed->setInputWidgetProperty("step", 0.001);
    zMaxSpeed->setInputWidgetProperty("minimum", 1);
    zMaxSpeed->setInputWidgetProperty("maximum", 1000);
    zMaxSpeed->bindLaserRegister(48);

    ConfigItem* zMaxAcceleration = group->addConfigItem(
        "zMaxAcceleration",
        tr("Z Max speed"),
        tr("Z Max speed"),
        30,
        DT_REAL
    );
    zMaxAcceleration->setInputWidgetType(IWT_FloatEditSlider);
    zMaxAcceleration->setInputWidgetProperty("step", 0.001);
    zMaxAcceleration->setInputWidgetProperty("minimum", 1);
    zMaxAcceleration->setInputWidgetProperty("maximum", 1000);
    zMaxAcceleration->bindLaserRegister(49);

    ConfigItem* zUrgentAcceleration = group->addConfigItem(
        "zUrgentAcceleration",
        tr("Z Urgent speed"),
        tr("Z Urgent speed"),
        30,
        DT_REAL
    );
    zUrgentAcceleration->setInputWidgetType(IWT_FloatEditSlider);
    zUrgentAcceleration->setInputWidgetProperty("step", 0.001);
    zUrgentAcceleration->setInputWidgetProperty("minimum", 1);
    zUrgentAcceleration->setInputWidgetProperty("maximum", 1000);
    zUrgentAcceleration->bindLaserRegister(50);

    ConfigItem* laserMaxPower = group->addConfigItem(
        "laserMaxPower",
        tr("Laser Max Power"),
        tr("Laser Max Power"),
        700,
        DT_REAL
    );
    laserMaxPower->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 10));
        }
    );
    laserMaxPower->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 10.0);
        }
    );
    laserMaxPower->setInputWidgetProperty("textTemplate", "%1%");
    laserMaxPower->setInputWidgetProperty("minimum", 1);
    laserMaxPower->setInputWidgetProperty("maximum", 100);
    laserMaxPower->bindLaserRegister(51);

    ConfigItem* laserMinPower = group->addConfigItem(
        "laserMinPower",
        tr("Laser Min Power"),
        tr("Laser Min Power"),
        700,
        DT_INT
    );
    laserMinPower->setInputWidgetType(IWT_FloatEditSlider);
    laserMinPower->setInputWidgetProperty("textTemplate", "%1%");
    laserMinPower->setInputWidgetProperty("minimum", 1);
    laserMinPower->setInputWidgetProperty("maximum", 100);
    laserMinPower->bindLaserRegister(51);

    ConfigItem* laserPowerFreq = group->addConfigItem(
        "laserPowerFreq",
        tr("Laser Min Power"),
        tr("Laser Min Power"),
        10,
        DT_REAL
    );
    laserPowerFreq->setInputWidgetProperty("step", 0.1);
    laserPowerFreq->setInputWidgetProperty("minimum", 1);
    laserPowerFreq->setInputWidgetProperty("maximum", 100);
    laserPowerFreq->bindLaserRegister(51);
}

QList<ConfigItemGroup*> Config::getGroups()
{
    return groups;
}
