#include "Config.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QTimer>

#include "ConfigItem.h"
#include "exception/LaserException.h"
#include "laser/LaserRegister.h"
#include "util/WidgetUtils.h"
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
    if (configFile.exists())
    {
        if (!configFile.open(QFile::Text | QFile::ReadOnly))
        {
            //QMessageBox::warning(nullptr, QObject::tr("Open Failure"), QObject::tr("An error occured when opening configuration file!"));
            configFile.close();
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
    }
    else
    {
        configFile.close();
        qLogD << "No valid config.json file found! We will create one.";
        save();
    }
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

    QStringList currentDisplayLanguages = QLocale::system().uiLanguages();
    QLocale displayLocale = QLocale(currentDisplayLanguages.first());
    ConfigItem* language = group->addConfigItem(
        "language"
        , tr("Language")
        , tr("Language for both UI and Business.")
        , displayLocale.language()
    );
    language->setInputWidgetType(IWT_ComboBox);
    language->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (!comboBox)
                return;

            comboBox->addItem("English", static_cast<int>(QLocale::English));
            comboBox->addItem("Chinese", static_cast<int>(QLocale::Chinese));

            QTimer::singleShot(0, 
                [=]() {
                    int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
                    comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
                }
            );
        }
    );

    ConfigItem* unit = group->addConfigItem(
        "unit"
        , tr("Unit")
        , tr("Global unit")
        , static_cast<int>(SU_MM)
        , DT_INT
    );
    unit->setInputWidgetType(IWT_ComboBox);
    unit->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (!comboBox)
                return;

            comboBox->addItem(tr("mm"), 4);

            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
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

    ConfigItem* gridContrast = group->addConfigItem(
        "gridContrast",
        tr("Grid Contrast"),
        tr("Grid contrast"),
        2,
        DT_INT
    );
    gridContrast->setInputWidgetType(IWT_ComboBox);
    gridContrast->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (!comboBox)
                return;

            comboBox->addItem(tr("Off"), 0);
            comboBox->addItem(tr("Low Contrast"), 1);
            comboBox->addItem(tr("Medium Contrast"), 2);
            comboBox->addItem(tr("High Contrast"), 3);

            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );

	ConfigItem* gridShapeDistance = group->addConfigItem(
		"gridShapeDistance",
		tr("Grid Shape Distance"),
		tr("(Pixel)"),
		3
	);
	gridShapeDistance->setInputWidgetProperty("minimum", 0);
	gridShapeDistance->setInputWidgetProperty("maximum", 10);

	ConfigItem* objectShapeDistance = group->addConfigItem(
		"objectShapeDistance",
		tr("Object Shape Distance"),
		tr("(Pixel)"),
		5
	);
	objectShapeDistance->setInputWidgetProperty("minimum", 0);
	objectShapeDistance->setInputWidgetProperty("maximum", 10);

	ConfigItem* clickSelectiontTolerance = group->addConfigItem(
		"clickSelectiontTolerance",
		tr("Click-selectiont Tolerance"),
		tr("(Pixel)"),
		5
	);
	clickSelectiontTolerance->setInputWidgetProperty("minimum", 0);
	clickSelectiontTolerance->setInputWidgetProperty("maximum", 10);

	
	ConfigItem* visualGridSpacing = group->addConfigItem(
		"visualGridSpacing",
		tr("Visual Grid Spacing"),
		tr("(mm)"),
		10
	);
	visualGridSpacing->setInputWidgetProperty("minimum", 0);
	visualGridSpacing->setInputWidgetProperty("maximum", 10);
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
        100,
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

            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );
    accMode->bindLaserRegister(1, false);

    ConfigItem* cuttingMoveSpeed = group->addConfigItem(
        "cuttingMoveSpeed",
        tr("Cutting Move Speed(mm/s)"),
        tr("Cutting move speed"),
        15,
        DT_REAL
    );
    cuttingMoveSpeed->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    cuttingMoveSpeed->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    //cuttingMoveSpeed->setInputWidgetProperty("textTemplate", "%1mm/s");
    cuttingMoveSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    cuttingMoveSpeed->setInputWidgetProperty("step", 0.001);
    cuttingMoveSpeed->setInputWidgetProperty("page", 10);
    cuttingMoveSpeed->setInputWidgetProperty("minimum", 0.001);
    cuttingMoveSpeed->setInputWidgetProperty("maximum", 100000);
    cuttingMoveSpeed->bindLaserRegister(2, false);

    ConfigItem* cuttingMoveAcc = group->addConfigItem(
        "cuttingMoveAcc",
        tr("Cutting Move Acceleration(mm/s<sup>2</sup>)"),
        tr("Cutting Move Acceleration"),
        45,
        DT_REAL
    );
    cuttingMoveAcc->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    cuttingMoveAcc->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    //cuttingMoveAcc->setInputWidgetProperty("textTemplate", "%1mm/s2");
    cuttingMoveAcc->setInputWidgetProperty("maximumLineEditWidth", 75);
    cuttingMoveAcc->setInputWidgetProperty("step", 0.001);
    cuttingMoveAcc->setInputWidgetProperty("page", 10);
    cuttingMoveAcc->setInputWidgetProperty("minimum", 0.001);
    cuttingMoveAcc->setInputWidgetProperty("maximum", 10000);
    cuttingMoveAcc->bindLaserRegister(3, false);

    ConfigItem* cuttingTurnSpeed = group->addConfigItem(
        "cuttingTurnSpeed",
        tr("Cutting Turn Speed(mm/s)"),
        tr("Cutting turn speed"),
        15,
        DT_REAL
    );
    cuttingTurnSpeed->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    cuttingTurnSpeed->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    //cuttingTurnSpeed->setInputWidgetProperty("textTemplate", "%1mm/s");
    cuttingTurnSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    cuttingTurnSpeed->setInputWidgetProperty("step", 0.001);
    cuttingTurnSpeed->setInputWidgetProperty("page", 10);
    cuttingTurnSpeed->setInputWidgetProperty("minimum", 1);
    cuttingTurnSpeed->setInputWidgetProperty("maximum", 1000);
    cuttingTurnSpeed->bindLaserRegister(4, false);

    ConfigItem* cuttingTurnAcc = group->addConfigItem(
        "cuttingTurnAcc",
        tr("Cutting Turn Acceleration(mm/s<sup>2</sup>)"),
        tr("Cutting turn acceleration"),
        45,
        DT_REAL
    );
    cuttingTurnAcc->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    cuttingTurnAcc->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    //cuttingTurnAcc->setInputWidgetProperty("textTemplate", "%1mm/s2");
    cuttingTurnAcc->setInputWidgetProperty("maximumLineEditWidth", 75);
    cuttingTurnAcc->setInputWidgetProperty("step", 0.001);
    cuttingTurnAcc->setInputWidgetProperty("page", 10);
    cuttingTurnAcc->setInputWidgetProperty("minimum", 1);
    cuttingTurnAcc->setInputWidgetProperty("maximum", 1000);
    cuttingTurnAcc->bindLaserRegister(5, false);

    ConfigItem* cuttingWorkAcc = group->addConfigItem(
        "cuttingWorkAcc",
        tr("Cutting Work Acceleration(mm/s<sup>2</sup>)"),
        tr("Cutting Work acceleration"),
        60,
        DT_REAL
    );
    cuttingWorkAcc->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    cuttingWorkAcc->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    //cuttingWorkAcc->setInputWidgetProperty("textTemplate", "%1mm/s2");
    cuttingWorkAcc->setInputWidgetProperty("maximumLineEditWidth", 75);
    cuttingWorkAcc->setInputWidgetProperty("step", 0.001);
    cuttingWorkAcc->setInputWidgetProperty("page", 10);
    cuttingWorkAcc->setInputWidgetProperty("minimum", 1);
    cuttingWorkAcc->setInputWidgetProperty("maximum", 10000);
    cuttingWorkAcc->bindLaserRegister(6, false);

    ConfigItem* cuttingMoveSpeedFactor = group->addConfigItem(
        "cuttingMoveSpeedFactor",
        tr("Cutting Move Speed Factor"),
        tr("Cutting move speed factor"),
        2
    );
    cuttingMoveSpeedFactor->setInputWidgetProperty("maximumLineEditWidth", 75);
    cuttingMoveSpeedFactor->setInputWidgetProperty("minimum", 1);
    cuttingMoveSpeedFactor->setInputWidgetProperty("maximum", 100);
    cuttingMoveSpeedFactor->bindLaserRegister(7, false);

    ConfigItem* cuttingWorkSpeedFactor = group->addConfigItem(
        "cuttingWorkSpeedFactor",
        tr("Cutting Work Speed Factor"),
        tr("Cutting Work speed factor"),
        2
    );
    cuttingWorkSpeedFactor->setInputWidgetProperty("maximumLineEditWidth", 75);
    cuttingWorkSpeedFactor->setInputWidgetProperty("minimum", 1);
    cuttingWorkSpeedFactor->setInputWidgetProperty("maximum", 100);
    cuttingWorkSpeedFactor->bindLaserRegister(8, false);

    ConfigItem* cuttingSpotSize = group->addConfigItem(
        "cuttingSpotSize",
        tr("Cutting Spot Size"),
        tr("Cutting spot size"),
        30
    );
    cuttingSpotSize->setInputWidgetProperty("maximumLineEditWidth", 75);
    cuttingSpotSize->setInputWidgetProperty("minimum", 1);
    cuttingSpotSize->setInputWidgetProperty("maximum", 1000);
    cuttingSpotSize->bindLaserRegister(9, false);

    ConfigItem* scanXStartSpeed = group->addConfigItem(
        "scanXStartSpeed",
        tr("Scan X Start Speed(mm/s)"),
        tr("Scan x start speed"),
        15,
        DT_REAL
    );
    scanXStartSpeed->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    scanXStartSpeed->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    //scanXStartSpeed->setInputWidgetProperty("textTemplate", "%1mm/s");
    scanXStartSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    scanXStartSpeed->setInputWidgetProperty("step", 0.001);
    scanXStartSpeed->setInputWidgetProperty("page", 10);
    scanXStartSpeed->setInputWidgetProperty("minimum", 1);
    scanXStartSpeed->setInputWidgetProperty("maximum", 10000);
    scanXStartSpeed->bindLaserRegister(10, false);

    ConfigItem* scanYStartSpeed = group->addConfigItem(
        "scanYStartSpeed",
        tr("Scan Y Start Speed(mm/s)"),
        tr("Scan y start speed"),
        15,
        DT_REAL
    );
    scanYStartSpeed->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    scanYStartSpeed->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    //scanYStartSpeed->setInputWidgetProperty("textTemplate", "%1mm/s");
    scanYStartSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    scanYStartSpeed->setInputWidgetProperty("step", 0.001);
    scanYStartSpeed->setInputWidgetProperty("page", 10);
    scanYStartSpeed->setInputWidgetProperty("minimum", 1);
    scanYStartSpeed->setInputWidgetProperty("maximum", 10000);
    scanYStartSpeed->bindLaserRegister(11, false);

    ConfigItem* scanXAcc = group->addConfigItem(
        "scanXAcc",
        tr("Scan X Acceleration(mm/s<sup>2</sup>)"),
        tr("Scan x acceleration"),
        5,
        DT_REAL
    );
    scanXAcc->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    scanXAcc->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    //scanXAcc->setInputWidgetProperty("textTemplate", "%1mm/s2");
    scanXAcc->setInputWidgetProperty("maximumLineEditWidth", 75);
    scanXAcc->setInputWidgetProperty("step", 0.001);
    scanXAcc->setInputWidgetProperty("page", 10);
    scanXAcc->setInputWidgetProperty("minimum", 1);
    scanXAcc->setInputWidgetProperty("maximum", 10000);
    scanXAcc->bindLaserRegister(12, false);

    ConfigItem* scanYAcc = group->addConfigItem(
        "scanYAcc",
        tr("Scan Y Acceleration(mm/s<sup>2</sup>)"),
        tr("Scan y acceleration"),
        45,
        DT_REAL
    );
    scanYAcc->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    scanYAcc->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    //scanYAcc->setInputWidgetProperty("textTemplate", "%1mm/s2");
    scanYAcc->setInputWidgetProperty("maximumLineEditWidth", 75);
    scanYAcc->setInputWidgetProperty("step", 0.001);
    scanYAcc->setInputWidgetProperty("page", 10);
    scanYAcc->setInputWidgetProperty("minimum", 1);
    scanYAcc->setInputWidgetProperty("maximum", 10000);
    scanYAcc->bindLaserRegister(13, false);

    ConfigItem* scanRowSpeed = group->addConfigItem(
        "scanRowSpeed",
        tr("Scan Row Speed(mm/s)"),
        tr("Scan row speed"),
        15,
        DT_REAL
    );
    scanRowSpeed->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    scanRowSpeed->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    //scanRowSpeed->setInputWidgetProperty("textTemplate", "%1mm/s");
    scanRowSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    scanRowSpeed->setInputWidgetProperty("step", 0.001);
    scanRowSpeed->setInputWidgetProperty("page", 10);
    scanRowSpeed->setInputWidgetProperty("minimum", 1);
    scanRowSpeed->setInputWidgetProperty("maximum", 10000);
    scanRowSpeed->bindLaserRegister(14, false);

    ConfigItem* scanRowInterval = group->addConfigItem(
        "scanRowInterval",
        tr("Scan Row Interval(mm)"),
        tr("Scan row interval"),
        0.007,
        DT_REAL
    );
    scanRowInterval->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    scanRowInterval->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    scanRowInterval->setInputWidgetProperty("maximumLineEditWidth", 75);
    scanRowInterval->setInputWidgetProperty("step", 0.001);
    scanRowInterval->setInputWidgetProperty("page", 0.01);
    scanRowInterval->setInputWidgetProperty("minimum", 0);
    scanRowInterval->setInputWidgetProperty("maximum", 1);
    scanRowInterval->bindLaserRegister(15, false);

    ConfigItem* scanReturnError = group->addConfigItem(
        "scanReturnError",
        tr("Scan Return Error(mm/s)"),
        tr("Scan return error"),
        0
    );
    //scanReturnError->setInputWidgetProperty("textTemplate", "%1mm/s");
    scanReturnError->setInputWidgetProperty("maximumLineEditWidth", 75);
    scanReturnError->setInputWidgetProperty("step", 0.001);
    scanReturnError->setInputWidgetProperty("page", 10);
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
    scanLaserPower->setInputWidgetProperty("textTemplate", "%1%");
    scanLaserPower->setInputWidgetProperty("maximumLineEditWidth", 75);
    scanLaserPower->setInputWidgetProperty("step", 0.1);
    scanLaserPower->setInputWidgetProperty("page", 10);
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
    scanReturnPos->setInputWidgetProperty("maximumLineEditWidth", 75);
    scanReturnPos->setInputWidgetProperty("page", 1000);
    scanReturnPos->setInputWidgetProperty("minimum", 0);
    scanReturnPos->setInputWidgetProperty("maximum", 100000);
    scanReturnPos->bindLaserRegister(21, false);

    ConfigItem* backlashXInterval = group->addConfigItem(
        "backlashXInterval",
        tr("Backlash X Interval(mm/s)"),
        tr("Backlash x interval"),
        0,
        DT_REAL
    );
    backlashXInterval->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    backlashXInterval->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    backlashXInterval->setInputWidgetProperty("maximumLineEditWidth", 75);
    backlashXInterval->setInputWidgetProperty("step", 0.001);
    backlashXInterval->setInputWidgetProperty("page", 10);
    backlashXInterval->setInputWidgetProperty("minimum", 1);
    backlashXInterval->setInputWidgetProperty("maximum", 100);
    backlashXInterval->bindLaserRegister(22, false);

    ConfigItem* backlashYInterval = group->addConfigItem(
        "backlashYInterval",
        tr("Backlash Y Interval(mm/s)"),
        tr("Backlash y interval"),
        0,
        DT_REAL
    );
    backlashYInterval->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    backlashYInterval->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    backlashYInterval->setInputWidgetProperty("maximumLineEditWidth", 75);
    backlashYInterval->setInputWidgetProperty("step", 0.001);
    backlashYInterval->setInputWidgetProperty("page", 10);
    backlashYInterval->setInputWidgetProperty("minimum", 1);
    backlashYInterval->setInputWidgetProperty("maximum", 100);
    backlashYInterval->bindLaserRegister(23, false);

    ConfigItem* backlashZInterval = group->addConfigItem(
        "backlashZInterval",
        tr("Backlash Z Interval(mm/s)"),
        tr("Backlash z interval"),
        0,
        DT_REAL
    );
    backlashZInterval->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    backlashZInterval->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    backlashZInterval->setInputWidgetProperty("maximumLineEditWidth", 75);
    backlashZInterval->setInputWidgetProperty("step", 0.001);
    backlashZInterval->setInputWidgetProperty("page", 10);
    backlashZInterval->setInputWidgetProperty("minimum", 1);
    backlashZInterval->setInputWidgetProperty("maximum", 100);
    backlashZInterval->bindLaserRegister(24, false);
}

void Config::loadSystemRegisters()
{
    ConfigItemGroup* group = new Config::SystemRegister;
    Config::SystemRegister::group = group;

    ConfigItem* sysRunTime = group->addConfigItem(
        "sysRunTime",
        tr("System Run Time"),
        tr("System run time"),
        0,
        DT_INT
    );
    sysRunTime->setReadOnly(true);
    sysRunTime->setInputWidgetType(IWT_LineEdit);
    sysRunTime->bindLaserRegister(9);

    ConfigItem* laserRunTime = group->addConfigItem(
        "laserRunTime",
        tr("Laser Run Time"),
        tr("Laser run time"),
        0,
        DT_INT
    );
    laserRunTime->setReadOnly(true);
    laserRunTime->setInputWidgetType(IWT_LineEdit);
    laserRunTime->bindLaserRegister(10);

    ConfigItem* sysRunNum = group->addConfigItem(
        "sysRunNum",
        tr("System Run Times"),
        tr("System run times"),
        0,
        DT_INT
    );
    sysRunNum->setReadOnly(true);
    sysRunNum->setInputWidgetType(IWT_LineEdit);
    sysRunNum->bindLaserRegister(11);

    ConfigItem* xMaxLength = group->addConfigItem(
        "xMaxLength",
        tr("X Max Length(mm)"),
        tr("X max length"),
        320,
        DT_REAL
    );
    xMaxLength->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    xMaxLength->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    //xMaxLength->setInputWidgetProperty("textTemplate", "%1mm");
    xMaxLength->setInputWidgetProperty("maximumLineEditWidth", 75);
    xMaxLength->setInputWidgetProperty("step", 0.001);
    xMaxLength->setInputWidgetProperty("page", 1);
    xMaxLength->setInputWidgetProperty("minimum", 1);
    xMaxLength->setInputWidgetProperty("maximum", 5000);
    xMaxLength->setInputWidgetProperty("decimals", 3);
    xMaxLength->bindLaserRegister(12);

    ConfigItem* xDirPhase = group->addConfigItem(
        "xDirPhase",
        tr("X Dir Phase"),
        tr("X dir phase"),
        1,
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

            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );
    xDirPhase->bindLaserRegister(13);

    ConfigItem* xLimitPhase = group->addConfigItem(
        "xLimitPhase",
        tr("X Limit Phase"),
        tr("X limit phase"),
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

            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );
    xLimitPhase->bindLaserRegister(14);

    ConfigItem* xZeroDev = group->addConfigItem(
        "xZeroDev",
        tr("X Zero Dev(mm)"),
        tr("X zero dev"),
        2,
        DT_REAL
    );
    xZeroDev->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    xZeroDev->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    //xZeroDev->setInputWidgetProperty("textTemplate", "%1mm");
    xZeroDev->setInputWidgetProperty("maximumLineEditWidth", 75);
    xZeroDev->setInputWidgetProperty("step", 0.001);
    xZeroDev->setInputWidgetProperty("page", 1);
    xZeroDev->setInputWidgetProperty("minimum", 0);
    xZeroDev->setInputWidgetProperty("maximum", 10);
    xZeroDev->setInputWidgetProperty("decimals", 3);
    xZeroDev->bindLaserRegister(15);

    ConfigItem* xStepLength = group->addConfigItem(
        "xStepLength",
        tr("X Step Length(mm)"),
        tr("X step length"),
        3.164557,
        DT_REAL
    );
    xStepLength->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000000));
        }
    );
    xStepLength->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000000.0);
        }
    );
    //xStepLength->setInputWidgetProperty("textTemplate", "%1mm");
    xStepLength->setInputWidgetProperty("maximumLineEditWidth", 75);
    xStepLength->setInputWidgetProperty("step", 0.000001);
    xStepLength->setInputWidgetProperty("page", 0.001);
    xStepLength->setInputWidgetProperty("minimum", 0.000001);
    xStepLength->setInputWidgetProperty("maximum", 100);
    xStepLength->setInputWidgetProperty("decimals", 6);
    xStepLength->bindLaserRegister(16);

    ConfigItem* xLimitNum = group->addConfigItem(
        "xLimitNum",
        tr("X Limit number"),
        tr("X limit number"),
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
            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );
    xLimitNum->bindLaserRegister(17);

    ConfigItem* xResetEnabled = group->addConfigItem(
        "xResetEnabled",
        tr("X Reset Enabled"),
        tr("X reset enabled"),
        true,
        DT_BOOL
    );
    xResetEnabled->bindLaserRegister(18);

    ConfigItem* xMotorNum = group->addConfigItem(
        "xMotorNum",
        tr("X Motor number"),
        tr("X motor number"),
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
            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );
    xMotorNum->bindLaserRegister(19);

    ConfigItem* xMotorCurrent = group->addConfigItem(
        "xMotorCurrent",
        tr("X Motor current"),
        tr("X motor current"),
        50,
        DT_REAL
    );
    xMotorCurrent->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 10));
        }
    );
    xMotorCurrent->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 10.0);
        }
    );
    xMotorCurrent->setInputWidgetProperty("textTemplate", "%1%");
    xMotorCurrent->setInputWidgetProperty("maximumLineEditWidth", 75);
    xMotorCurrent->setInputWidgetProperty("step", 0.1);
    xMotorCurrent->setInputWidgetProperty("page", 10);
    xMotorCurrent->setInputWidgetProperty("minimum", 0.1);
    xMotorCurrent->setInputWidgetProperty("maximum", 100);
    xMotorCurrent->bindLaserRegister(20);

    ConfigItem* xStartSpeed = group->addConfigItem(
        "xStartSpeed",
        tr("X Start speed(mm/s)"),
        tr("X start speed"),
        15,
        DT_REAL
    );
    xStartSpeed->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    xStartSpeed->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    //xStartSpeed->setInputWidgetProperty("textTemplate", "%1mm/s");
    xStartSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    xStartSpeed->setInputWidgetProperty("step", 0.001);
    xStartSpeed->setInputWidgetProperty("page", 10);
    xStartSpeed->setInputWidgetProperty("minimum", 0.001);
    xStartSpeed->setInputWidgetProperty("maximum", 1000);
    xStartSpeed->setInputWidgetProperty("decimals", 3);
    xStartSpeed->bindLaserRegister(21);

    ConfigItem* xMaxSpeed = group->addConfigItem(
        "xMaxSpeed",
        tr("X Max speed(mm/s)"),
        tr("X max speed"),
        45,
        DT_REAL
    );
    xMaxSpeed->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    xMaxSpeed->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    //xMaxSpeed->setInputWidgetProperty("textTemplate", "%1mm/s2");
    xMaxSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    xMaxSpeed->setInputWidgetProperty("step", 0.001);
    xMaxSpeed->setInputWidgetProperty("page", 10);
    xMaxSpeed->setInputWidgetProperty("minimum", 0.001);
    xMaxSpeed->setInputWidgetProperty("maximum", 1000);
    xMaxSpeed->setInputWidgetProperty("decimals", 3);
    xMaxSpeed->bindLaserRegister(22);

    ConfigItem* xMaxAcceleration = group->addConfigItem(
        "xMaxAcceleration",
        tr("X Max Acceleration(mm/s<sup>2</sup>)"),
        tr("X max acceleration"),
        45,
        DT_REAL
    );
    xMaxAcceleration->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    xMaxAcceleration->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    //xMaxAcceleration->setInputWidgetProperty("textTemplate", "%1mm/s2");
    xMaxAcceleration->setInputWidgetProperty("maximumLineEditWidth", 75);
    xMaxAcceleration->setInputWidgetProperty("step", 0.001);
    xMaxAcceleration->setInputWidgetProperty("page", 10);
    xMaxAcceleration->setInputWidgetProperty("minimum", 1);
    xMaxAcceleration->setInputWidgetProperty("maximum", 1000);
    xMaxAcceleration->setInputWidgetProperty("decimals", 3);
    xMaxAcceleration->bindLaserRegister(23);

    ConfigItem* xUrgentAcceleration = group->addConfigItem(
        "xUrgentAcceleration",
        tr("X Urgent Acceleration(mm/s<sup>2</sup>)"),
        tr("X urgent acceleration"),
        45,
        DT_REAL
    );
    xUrgentAcceleration->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    xUrgentAcceleration->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    //xUrgentAcceleration->setInputWidgetProperty("textTemplate", "%1mm/s2");
    xUrgentAcceleration->setInputWidgetProperty("maximumLineEditWidth", 75);
    xUrgentAcceleration->setInputWidgetProperty("step", 0.001);
    xUrgentAcceleration->setInputWidgetProperty("page", 10);
    xUrgentAcceleration->setInputWidgetProperty("minimum", 1);
    xUrgentAcceleration->setInputWidgetProperty("maximum", 1000);
    xUrgentAcceleration->setInputWidgetProperty("decimals", 3);
    xUrgentAcceleration->bindLaserRegister(24);

    ConfigItem* yMaxLength = group->addConfigItem(
        "yMaxLength",
        tr("Y Max Length(mm)"),
        tr("Y max length"),
        200,
        DT_REAL
    );
    yMaxLength->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    yMaxLength->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    //xMaxLength->setInputWidgetProperty("textTemplate", "%1mm");
    yMaxLength->setInputWidgetProperty("maximumLineEditWidth", 75);
    yMaxLength->setInputWidgetProperty("step", 0.001);
    yMaxLength->setInputWidgetProperty("page", 1);
    yMaxLength->setInputWidgetProperty("minimum", 1);
    yMaxLength->setInputWidgetProperty("maximum", 5000);
    yMaxLength->setInputWidgetProperty("decimals", 3);
    yMaxLength->bindLaserRegister(25);

    ConfigItem* yDirPhase = group->addConfigItem(
        "yDirPhase",
        tr("Y Dir Phase"),
        tr("Y dir phase"),
        1,
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
            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );
    yDirPhase->bindLaserRegister(26);

    ConfigItem* yLimitPhase = group->addConfigItem(
        "yLimitPhase",
        tr("Y Limit Phase"),
        tr("Y limit phase"),
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
            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );
    yLimitPhase->bindLaserRegister(27);

    ConfigItem* yZeroDev = group->addConfigItem(
        "yZeroDev",
        tr("Y Zero Dev(mm)"),
        tr("Y zero dev"),
        2000,
        DT_REAL
    );
    yZeroDev->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    yZeroDev->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    //yZeroDev->setInputWidgetProperty("textTemplate", "%1mm");
    yZeroDev->setInputWidgetProperty("maximumLineEditWidth", 75);
    yZeroDev->setInputWidgetProperty("step", 0.001);
    yZeroDev->setInputWidgetProperty("page", 1);
    yZeroDev->setInputWidgetProperty("minimum", 1);
    yZeroDev->setInputWidgetProperty("maximum", 10);
    yZeroDev->setInputWidgetProperty("decimals", 3);
    yZeroDev->bindLaserRegister(28);

    ConfigItem* yStepLength = group->addConfigItem(
        "yStepLength",
        tr("Y Step Length(mm)"),
        tr("Y step length"),
        3.164557,
        DT_REAL
    );
    yStepLength->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000000));
        }
    );
    yStepLength->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000000.0);
        }
    );
    //yStepLength->setInputWidgetProperty("textTemplate", "%1mm");
    yStepLength->setInputWidgetProperty("maximumLineEditWidth", 75);
    yStepLength->setInputWidgetProperty("step", 0.000001);
    yStepLength->setInputWidgetProperty("page", 0.001);
    yStepLength->setInputWidgetProperty("minimum", 0.000001);
    yStepLength->setInputWidgetProperty("maximum", 100);
    yStepLength->setInputWidgetProperty("decimals", 6);
    yStepLength->bindLaserRegister(29);

    ConfigItem* yLimitNum = group->addConfigItem(
        "yLimitNum",
        tr("Y Limit number"),
        tr("Y limit number"),
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
            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );
    yLimitNum->bindLaserRegister(30);

    ConfigItem* yResetEnabled = group->addConfigItem(
        "yResetEnabled",
        tr("Y Reset Enabled"),
        tr("Y reset enabled"),
        true,
        DT_BOOL
    );
    yResetEnabled->bindLaserRegister(31);

    ConfigItem* yMotorNum = group->addConfigItem(
        "yMotorNum",
        tr("Y Motor number"),
        tr("Y motor number"),
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
            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );
    yMotorNum->bindLaserRegister(32);

    ConfigItem* yMotorCurrent = group->addConfigItem(
        "yMotorCurrent",
        tr("Y Motor current"),
        tr("Y motor current"),
        50,
        DT_REAL
    );
    yMotorCurrent->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 10));
        }
    );
    yMotorCurrent->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 10.0);
        }
    );
    //yMotorCurrent->setInputWidgetProperty("textTemplate", "%1%");
    yMotorCurrent->setInputWidgetProperty("maximumLineEditWidth", 75);
    yMotorCurrent->setInputWidgetProperty("step", 0.1);
    yMotorCurrent->setInputWidgetProperty("page", 10);
    yMotorCurrent->setInputWidgetProperty("minimum", 1);
    yMotorCurrent->setInputWidgetProperty("maximum", 100);
    yMotorCurrent->bindLaserRegister(33);

    ConfigItem* yStartSpeed = group->addConfigItem(
        "yStartSpeed",
        tr("Y Start speed(mm/s)"),
        tr("Y start speed"),
        15,
        DT_REAL
    );
    yStartSpeed->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    yStartSpeed->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    //yStartSpeed->setInputWidgetProperty("textTemplate", "%1mm/s");
    yStartSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    yStartSpeed->setInputWidgetProperty("step", 0.001);
    yStartSpeed->setInputWidgetProperty("page", 10);
    yStartSpeed->setInputWidgetProperty("minimum", 1);
    yStartSpeed->setInputWidgetProperty("maximum", 100);
    yStartSpeed->setInputWidgetProperty("decimals", 3);
    yStartSpeed->bindLaserRegister(34);

    ConfigItem* yMaxSpeed = group->addConfigItem(
        "yMaxSpeed",
        tr("Y Max speed(mm/s)"),
        tr("Y max speed"),
        45,
        DT_REAL
    );
    yMaxSpeed->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    yMaxSpeed->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    //yMaxSpeed->setInputWidgetProperty("textTemplate", "%1mm/s2");
    yMaxSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    yMaxSpeed->setInputWidgetProperty("step", 0.001);
    yMaxSpeed->setInputWidgetProperty("page", 10);
    yMaxSpeed->setInputWidgetProperty("minimum", 1);
    yMaxSpeed->setInputWidgetProperty("maximum", 1000);
    yMaxSpeed->setInputWidgetProperty("decimals", 3);
    yMaxSpeed->bindLaserRegister(35);

    ConfigItem* yMaxAcceleration = group->addConfigItem(
        "yMaxAcceleration",
        tr("Y Max Acceleration(mm/s<sup>2</sup>)"),
        tr("Y max acceleration"),
        45,
        DT_REAL
    );
    yMaxAcceleration->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    yMaxAcceleration->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    //yMaxAcceleration->setInputWidgetProperty("textTemplate", "%1mm/s2");
    yMaxAcceleration->setInputWidgetProperty("maximumLineEditWidth", 75);
    yMaxAcceleration->setInputWidgetProperty("step", 0.001);
    yMaxAcceleration->setInputWidgetProperty("page", 10);
    yMaxAcceleration->setInputWidgetProperty("minimum", 1);
    yMaxAcceleration->setInputWidgetProperty("maximum", 1000);
    yMaxAcceleration->setInputWidgetProperty("decimals", 3);
    yMaxAcceleration->bindLaserRegister(36);

    ConfigItem* yUrgentAcceleration = group->addConfigItem(
        "yUrgentAcceleration",
        tr("Y Urgent Acceleration(mm/s<sup>2</sup>)"),
        tr("Y urgent acceleration"),
        20,
        DT_REAL
    );
    yUrgentAcceleration->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    yUrgentAcceleration->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    //yUrgentAcceleration->setInputWidgetProperty("textTemplate", "%1mm/s2");
    yUrgentAcceleration->setInputWidgetProperty("maximumLineEditWidth", 75);
    yUrgentAcceleration->setInputWidgetProperty("step", 0.001);
    yUrgentAcceleration->setInputWidgetProperty("page", 10);
    yUrgentAcceleration->setInputWidgetProperty("minimum", 1);
    yUrgentAcceleration->setInputWidgetProperty("maximum", 1000);
    yUrgentAcceleration->setInputWidgetProperty("decimals", 3);
    yUrgentAcceleration->bindLaserRegister(37);

    ConfigItem* zMaxLength = group->addConfigItem(
        "zMaxLength",
        tr("Z Max Length(mm)"),
        tr("Z max length"),
        200,
        DT_INT
    );
    zMaxLength->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    zMaxLength->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    //xMaxLength->setInputWidgetProperty("textTemplate", "%1mm");
    zMaxLength->setInputWidgetProperty("maximumLineEditWidth", 75);
    zMaxLength->setInputWidgetProperty("step", 0.001);
    zMaxLength->setInputWidgetProperty("page", 1);
    zMaxLength->setInputWidgetProperty("minimum", 1);
    zMaxLength->setInputWidgetProperty("maximum", 5000);
    zMaxLength->setInputWidgetProperty("decimals", 3);
    zMaxLength->bindLaserRegister(38);

    ConfigItem* zDirPhase = group->addConfigItem(
        "zDirPhase",
        tr("Z Dir Phase"),
        tr("Z dir phase"),
        1,
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
            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );
    zDirPhase->bindLaserRegister(39);

    ConfigItem* zLimitPhase = group->addConfigItem(
        "zLimitPhase",
        tr("Z Limit Length"),
        tr("Z limit length"),
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
            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );
    zLimitPhase->bindLaserRegister(40);

    ConfigItem* zZeroDev = group->addConfigItem(
        "zZeroDev",
        tr("Z Zero Dev(mm)"),
        tr("Z zero dev"),
        2,
        DT_REAL
    );
    zZeroDev->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    zZeroDev->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    //zZeroDev->setInputWidgetProperty("textTemplate", "%1mm");
    zZeroDev->setInputWidgetProperty("maximumLineEditWidth", 75);
    zZeroDev->setInputWidgetProperty("step", 0.001);
    zZeroDev->setInputWidgetProperty("page", 1);
    zZeroDev->setInputWidgetProperty("minimum", 1);
    zZeroDev->setInputWidgetProperty("maximum", 10);
    zZeroDev->setInputWidgetProperty("decimals", 3);
    zZeroDev->bindLaserRegister(41);

    ConfigItem* zStepLength = group->addConfigItem(
        "zStepLength",
        tr("Z Step Length(mm)"),
        tr("Z step length"),
        6.2,
        DT_REAL
    );
    zStepLength->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000000));
        }
    );
    zStepLength->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000000.0);
        }
    );
    //zStepLength->setInputWidgetProperty("textTemplate", "%1mm");
    zStepLength->setInputWidgetProperty("maximumLineEditWidth", 75);
    zStepLength->setInputWidgetProperty("step", 0.000001);
    zStepLength->setInputWidgetProperty("page", 0.001);
    zStepLength->setInputWidgetProperty("minimum", 0.000001);
    zStepLength->setInputWidgetProperty("maximum", 100);
    zStepLength->setInputWidgetProperty("decimals", 6);
    zStepLength->bindLaserRegister(42);

    ConfigItem* zLimitNum = group->addConfigItem(
        "zLimitNum",
        tr("Z Limit number"),
        tr("Z limit number"),
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
            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );
    zLimitNum->bindLaserRegister(43);

    ConfigItem* zResetEnabled = group->addConfigItem(
        "zResetEnabled",
        tr("Z Reset Enabled"),
        tr("Z reset enabled"),
        true,
        DT_BOOL
    );
    zResetEnabled->bindLaserRegister(44);

    ConfigItem* zMotorNum = group->addConfigItem(
        "zMotorNum",
        tr("Z Motor number"),
        tr("Z motor number"),
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
            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );
    zMotorNum->bindLaserRegister(45);

    ConfigItem* zMotorCurrent = group->addConfigItem(
        "zMotorCurrent",
        tr("Z Motor current"),
        tr("Z motor current"),
        50,
        DT_REAL
    );
    zMotorCurrent->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 10));
        }
    );
    zMotorCurrent->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 10.0);
        }
    );
    zMotorCurrent->setInputWidgetProperty("textTemplate", "%1%");
    zMotorCurrent->setInputWidgetProperty("maximumLineEditWidth", 75);
    zMotorCurrent->setInputWidgetProperty("step", 0.1);
    zMotorCurrent->setInputWidgetProperty("page", 10);
    zMotorCurrent->setInputWidgetProperty("minimum", 1);
    zMotorCurrent->setInputWidgetProperty("maximum", 100);
    zMotorCurrent->bindLaserRegister(46);

    ConfigItem* zStartSpeed = group->addConfigItem(
        "zStartSpeed",
        tr("Z Start speed(mm/s)"),
        tr("Z start speed"),
        15,
        DT_REAL
    );
    zStartSpeed->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    zStartSpeed->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    //zStartSpeed->setInputWidgetProperty("textTemplate", "%1mm/s");
    zStartSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    zStartSpeed->setInputWidgetProperty("step", 0.001);
    zStartSpeed->setInputWidgetProperty("page", 10);
    zStartSpeed->setInputWidgetProperty("minimum", 1);
    zStartSpeed->setInputWidgetProperty("maximum", 100);
    zStartSpeed->setInputWidgetProperty("decimals", 3);
    zStartSpeed->bindLaserRegister(47);

    ConfigItem* zMaxSpeed = group->addConfigItem(
        "zMaxSpeed",
        tr("Z Max speed(mm/s)"),
        tr("Z max speed"),
        10,
        DT_REAL
    );
    zMaxSpeed->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    zMaxSpeed->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    //zMaxSpeed->setInputWidgetProperty("textTemplate", "%1mm/s2");
    zMaxSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    zMaxSpeed->setInputWidgetProperty("step", 0.001);
    zMaxSpeed->setInputWidgetProperty("page", 10);
    zMaxSpeed->setInputWidgetProperty("minimum", 1);
    zMaxSpeed->setInputWidgetProperty("maximum", 1000);
    zMaxSpeed->setInputWidgetProperty("decimals", 3);
    zMaxSpeed->bindLaserRegister(48);

    ConfigItem* zMaxAcceleration = group->addConfigItem(
        "zMaxAcceleration",
        tr("Z Max Acceleration(mm/s<sup>2</sup>)"),
        tr("Z max acceleration"),
        30,
        DT_REAL
    );
    zMaxAcceleration->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    zMaxAcceleration->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    //zMaxAcceleration->setInputWidgetProperty("textTemplate", "%1mm/s2");
    zMaxAcceleration->setInputWidgetProperty("maximumLineEditWidth", 75);
    zMaxAcceleration->setInputWidgetProperty("step", 0.001);
    zMaxAcceleration->setInputWidgetProperty("page", 10);
    zMaxAcceleration->setInputWidgetProperty("minimum", 1);
    zMaxAcceleration->setInputWidgetProperty("maximum", 1000);
    zMaxAcceleration->setInputWidgetProperty("decimals", 3);
    zMaxAcceleration->bindLaserRegister(49);

    ConfigItem* zUrgentAcceleration = group->addConfigItem(
        "zUrgentAcceleration",
        tr("Z Urgent Acceleration(mm/s<sup>2</sup>)"),
        tr("Z urgent acceleration"),
        30,
        DT_REAL
    );
    zUrgentAcceleration->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    zUrgentAcceleration->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    //zUrgentAcceleration->setInputWidgetProperty("textTemplate", "%1mm/s2");
    zUrgentAcceleration->setInputWidgetProperty("maximumLineEditWidth", 75);
    zUrgentAcceleration->setInputWidgetProperty("step", 0.001);
    zUrgentAcceleration->setInputWidgetProperty("page", 10);
    zUrgentAcceleration->setInputWidgetProperty("minimum", 1);
    zUrgentAcceleration->setInputWidgetProperty("maximum", 1000);
    zUrgentAcceleration->setInputWidgetProperty("decimals", 3);
    zUrgentAcceleration->bindLaserRegister(50);

    ConfigItem* laserMaxPower = group->addConfigItem(
        "laserMaxPower",
        tr("Laser Max Power"),
        tr("Laser max power"),
        70,
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
    laserMaxPower->setInputWidgetProperty("maximumLineEditWidth", 75);
    laserMaxPower->setInputWidgetProperty("minimum", 1);
    laserMaxPower->setInputWidgetProperty("maximum", 100);
    laserMaxPower->bindLaserRegister(51, true, SS_DIRECTLY);

    ConfigItem* laserMinPower = group->addConfigItem(
        "laserMinPower",
        tr("Laser Min Power"),
        tr("Laser min power"),
        70,
        DT_REAL
    );
    laserMinPower->setSaveDataHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 10));
        }
    );
    laserMinPower->setLoadDataHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 10.0);
        }
    );
    laserMinPower->setInputWidgetProperty("textTemplate", "%1%");
    laserMinPower->setInputWidgetProperty("maximumLineEditWidth", 75);
    laserMinPower->setInputWidgetProperty("minimum", 1);
    laserMinPower->setInputWidgetProperty("maximum", 100);
    laserMinPower->bindLaserRegister(52, true, SS_DIRECTLY);

    ConfigItem* laserPowerFreq = group->addConfigItem(
        "laserPowerFreq",
        tr("Laser Min Power"),
        tr("Laser min power"),
        4000
    );
    laserPowerFreq->setInputWidgetProperty("maximumLineEditWidth", 75);
    laserPowerFreq->setInputWidgetProperty("page", 1000);
    laserPowerFreq->setInputWidgetProperty("minimum", 1);
    laserPowerFreq->setInputWidgetProperty("maximum", 10000);
    laserPowerFreq->bindLaserRegister(53, true, SS_DIRECTLY);

    ConfigItem* xPhaseEnabled = group->addConfigItem(
        "xPhaseEnabled",
        tr("X Phase Enabled"),
        tr("X phase enabled"),
        true,
        DT_BOOL
    );
    xPhaseEnabled->bindLaserRegister(54);

    ConfigItem* yPhaseEnabled = group->addConfigItem(
        "yPhaseEnabled",
        tr("Y Phase Enabled"),
        tr("Y phase enabled"),
        true,
        DT_BOOL
    );
    yPhaseEnabled->bindLaserRegister(55);

    ConfigItem* zPhaseEnabled = group->addConfigItem(
        "zPhaseEnabled",
        tr("Z Phase Enabled"),
        tr("Z phase enabled"),
        true,
        DT_BOOL
    );
    zPhaseEnabled->bindLaserRegister(54);
}

QList<ConfigItemGroup*> Config::getGroups()
{
    return groups;
}

void Config::refreshTranslation()
{
}
