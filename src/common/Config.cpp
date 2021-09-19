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
#include "widget/RadioButtonGroup.h"
#include "widget/SmallDiagonalLimitationWidget.h"

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
ConfigItemGroup* Config::Debug::group(nullptr);

Config::Config()
{

}

Config::~Config()
{
}

void Config::init()
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
    loadDebug();
}

void Config::load()
{
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
            group->confirm();
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
        [](QWidget* widget, ConfigItem* item, InputWidgetWrapper* wrapper)
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
        [](QWidget* widget, ConfigItem* item, InputWidgetWrapper* wrapper)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (!comboBox)
                return;

            comboBox->addItem(tr("mm"), 4);

            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );

    ConfigItem* machiningUnit = group->addConfigItem(
        "machiningUnit",
        tr("Machining Unit"),
        tr("Machining Unit"),
        1000,
        DT_REAL
    );
    machiningUnit->setInputWidgetType(IWT_DoubleSpinBox);
    machiningUnit->setInputWidgetProperty("minimum", 1);
    machiningUnit->setInputWidgetProperty("maximum", 1000);
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
        [](QWidget* widget, ConfigItem* item, InputWidgetWrapper* wrapper)
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

    ConfigItem* splitterHandleWidth = group->addConfigItem(
        "splitterHandleWidth",
        tr("Splitter Handle Width"),
        tr("Splitter Handle Width"),
        1
    );
    splitterHandleWidth->setInputWidgetProperty("minimum", 1);
    splitterHandleWidth->setInputWidgetProperty("maximum", 20);

    ConfigItem* autoRepeatDelay = group->addConfigItem(
        "autoRepeatDelay",
        tr("Auto repeat delay"),
        tr("Auto repeat delay when press down on a button"),
        1000
    );
    autoRepeatDelay->setInputWidgetProperty("minimum", 0);
    autoRepeatDelay->setInputWidgetProperty("maximum", 2000);

    ConfigItem* autoRepeatInterval = group->addConfigItem(
        "autoRepeatInterval",
        tr("Auto repeat interval"),
        tr("Auto repeat interval when pressing a button"),
        200
    );
    autoRepeatInterval->setInputWidgetProperty("minimum", 0);
    autoRepeatInterval->setInputWidgetProperty("maximum", 2000);
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

    ConfigItem* rowInterval = group->addConfigItem(
        "rowInterval",
        tr("Row interval"),
        tr("Row interval"),
        70,
        DT_INT
    );
    rowInterval->setInputWidgetProperty("minimum", 0);
    rowInterval->setInputWidgetProperty("maximum", 1000);
    rowInterval->setInputWidgetProperty("textTemplate", "%1μm");

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

    /*ConfigItem* maxGroupSize = group->addConfigItem(
        "maxGroupSize",
        tr("Max group size"),
        tr("Max children count in one group."),
        10
    );
    maxGroupSize->setInputWidgetProperty("minimum", 1);
    maxGroupSize->setInputWidgetProperty("maximum", 100);*/

    ConfigItem* groupingOrientation = group->addConfigItem(
        "groupingOrientation",
        tr("Grouping Orientation"),
        tr("Grouping orientation"),
        Qt::Vertical,
        DT_INT
    );
    groupingOrientation->setInputWidgetType(IWT_ComboBox);
    groupingOrientation->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item, InputWidgetWrapper* wrapper)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (!comboBox)
                return;

            comboBox->addItem(tr("Horizontal"), 1);
            comboBox->addItem(tr("Vertical"), 2);

            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );

    ConfigItem* maxGroupingGridSize = group->addConfigItem(
        "maxGroupingGridSize",
        tr("Max grouping grid size"),
        tr("Max grouping grid size."),
        30,
        DT_REAL
    );
    maxGroupingGridSize->setInputWidgetType(IWT_FloatEditSlider);
    maxGroupingGridSize->setInputWidgetProperty("minimum", 1.0);
    maxGroupingGridSize->setInputWidgetProperty("maximum", 1000.0);

    ConfigItem* searchingXYWeight = group->addConfigItem(
        "searchingXYWeight",
        tr("Searching XY Weight"),
        tr("Weight of xy element of laser point vector against angle element"),
        0.9,
        DT_REAL
    );
    searchingXYWeight->setInputWidgetType(IWT_FloatEditSlider);
    searchingXYWeight->setInputWidgetProperty("minimum", 0);
    searchingXYWeight->setInputWidgetProperty("maximum", 1);
    searchingXYWeight->setInputWidgetProperty("step", 0.01);
    searchingXYWeight->setInputWidgetProperty("page", 0.1);
    searchingXYWeight->setInputWidgetProperty("decimals", 2);
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
        40
    );
    maxIntervalDistance->setInputWidgetProperty("minimum", 1);
    maxIntervalDistance->setInputWidgetProperty("maximum", 1000);

    ConfigItem* maxStartingPoints = group->addConfigItem(
        "maxStartingPoints",
        tr("Max starting points"),
        tr("Max starting points."),
        8
    );
    maxStartingPoints->setInputWidgetProperty("minimum", 1);
    maxStartingPoints->setInputWidgetProperty("maximum", 20);

    ConfigItem* enableSmallDiagonal = group->addConfigItem(
        "enableSmallDiagonal",
        tr("Enable small diagonal"),
        tr("Enable small diagonal limitation"),
        false,
        DT_BOOL
    );
    enableSmallDiagonal->setStoreStrategy(SS_DIRECTLY);

    QVariant smallDiagonalLimitationVar;
    SmallDiagonalLimitation* limitation = new SmallDiagonalLimitation;
    smallDiagonalLimitationVar.setValue(limitation);
    qLogD << "small diagonal limitation type: " << smallDiagonalLimitationVar.userType();
    ConfigItem* smallDiagonalLimitation = group->addConfigItem(
        "smallDiagonalLimitation",
        tr("Small diagonal limitation"),
        tr("Small diagonal limitation"),
        smallDiagonalLimitationVar,
        DT_CUSTOM
    );
    smallDiagonalLimitation->setInputWidgetType(IWT_Custom);
    smallDiagonalLimitation->setCreateWidgetHook(
        [](ConfigItem* item) {
            return qobject_cast<QWidget*>(new SmallDiagonalLimitationWidget(item));
        }
    );
    smallDiagonalLimitation->setDestroyHook(
        [](ConfigItem* item) {
            SmallDiagonalLimitation* limitation = item->value().value<SmallDiagonalLimitation*>();
            delete limitation;
        }
    );
    smallDiagonalLimitation->setToJsonHook(
        [](const ConfigItem* item) {
            SmallDiagonalLimitation* limitation = item->value().value<SmallDiagonalLimitation*>();
            QJsonObject value = limitation->toJson();
            QJsonObject json;
            json["value"] = value;
            json["defaultValue"] = value;
            return json;
        }
    );
    smallDiagonalLimitation->setFromJsonHook(
        [=](QVariant& value, QVariant& defaultValue, const QJsonObject& json, ConfigItem* item) {
            if (json.contains("value"))
            {
                //void* ptr = item->value().value<void*>();
                //SmallDiagonalLimitation* limitation = item->value().value<SmallDiagonalLimitation*>();
                //SmallDiagonalLimitation* limitation = static_cast<SmallDiagonalLimitation*>(ptr);
                //if (limitation)
                limitation->fromJson(json["value"].toObject());
            }
        }
    );
    connect(enableSmallDiagonal, &ConfigItem::valueChanged,
        [=](const QVariant& value, ModifiedBy modifiedBy) {
            bool enabled = value.toBool();
            smallDiagonalLimitation->setEnabled(enabled);
        }
    );

    ConfigItem* relativePoint = group->addConfigItem(
        "relativePoint",
        tr("Relative Point"),
        tr("Relative Point"),
        false,
        DT_BOOL
    );
    relativePoint->setStoreStrategy(SS_DIRECTLY);
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

    ConfigItem* startFrom = group->addConfigItem(
        "startFrom",
        tr("Start From"),
        tr("Start From"),
        0,
        DT_INT
    );
    startFrom->setInputWidgetType(IWT_ComboBox);
    startFrom->setStoreStrategy(SS_DIRECTLY);
    startFrom->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item, InputWidgetWrapper* wrapper)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (!comboBox)
                return;

            comboBox->addItem(tr("Current Position"), SFT_CurrentPosition);
            comboBox->addItem(tr("User Origin"), SFT_UserOrigin);
            comboBox->addItem(tr("Absolute Coords"), SFT_AbsoluteCoords);

            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );

    ConfigItem* jobOrigin = group->addConfigItem(
        "jobOrigin",
        tr("Job Origin"),
        tr("Job Origin"),
        0,
        DT_INT
    );
    jobOrigin->setInputWidgetType(IWT_Custom);
    jobOrigin->setCreateWidgetHook(
        [=](ConfigItem* item) {
            return qobject_cast<QWidget*>(new RadioButtonGroup());
        }
    );
    jobOrigin->setWidgetInitializeHook(
        [=](QWidget* widget, ConfigItem* item, InputWidgetWrapper* wrapper) {
            RadioButtonGroup* radioGroup = qobject_cast<RadioButtonGroup*>(widget);
            QObject::connect(radioGroup, &RadioButtonGroup::valueChanged, wrapper, QOverload<int>::of(&InputWidgetWrapper::onValueChanged));
        }
    );
    jobOrigin->setUpdateWidgetValueHook(
        [=](QWidget* widget, const QVariant& value) {
            RadioButtonGroup* radioGroup = qobject_cast<RadioButtonGroup*>(widget);
            radioGroup->setValue(value.toInt());
        }
    );
    connect(startFrom, &ConfigItem::valueChanged,
        [=](const QVariant& value, ModifiedBy modifiedBy) {
            for (QWidget* widget : jobOrigin->boundedWidgets())
            {
                RadioButtonGroup* group = qobject_cast<RadioButtonGroup*>(widget);
                int index = value.toInt();
                switch (index)
                {
                case SFT_CurrentPosition:
                case SFT_UserOrigin:
                    group->setEnabled(true);
                    group->setRowsCols(3, 3);
                    break;
                case SFT_AbsoluteCoords:
                    group->setEnabled(false);
                    break;
                }
            }
        }
    );

    ConfigItem* xEnabled = group->addConfigItem(
        "xEnabled",
        tr("X Enabled"),
        tr("X Enabled"),
        true,
        DT_BOOL
    );
    xEnabled->setStoreStrategy(SS_DIRECTLY);

    ConfigItem* yEnabled = group->addConfigItem(
        "yEnabled",
        tr("Y Enabled"),
        tr("Y Enabled"),
        true,
        DT_BOOL
    );
    yEnabled->setStoreStrategy(SS_DIRECTLY);

    ConfigItem* zEnabled = group->addConfigItem(
        "zEnabled",
        tr("Z Enabled"),
        tr("Z Enabled"),
        true,
        DT_BOOL
    );
    zEnabled->setStoreStrategy(SS_DIRECTLY);
}

void Config::loadUserReigsters()
{
    ConfigItemGroup* group = new Config::UserRegister;
    Config::UserRegister::group = group;

    ConfigItem* head = group->addConfigItem(
        "head",
        tr("[00] Head Data"),
        tr("Head data for testing"),
        0x12345678,
        DT_INT
    );
    head->setReadOnly();
    head->setInputWidgetType(IWT_LineEdit);
    head->setInputWidgetProperty("readOnly", true);

    ConfigItem* accMode = group->addConfigItem(
        "accMode",
        tr("[01] Acceleration Mode"),
        tr("Acceleration mode"),
        0
    );
    accMode->setInputWidgetType(IWT_ComboBox);
    accMode->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item, InputWidgetWrapper* wrapper)
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

    ConfigItem* cuttingMoveSpeed = group->addConfigItem(
        "cuttingMoveSpeed",
        tr("[02] Cutting Move Speed(mm/s)"),
        tr("Cutting move speed"),
        15,
        DT_INT
    );
    cuttingMoveSpeed->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    cuttingMoveSpeed->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    cuttingMoveSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    cuttingMoveSpeed->setInputWidgetProperty("step", 0.001);
    cuttingMoveSpeed->setInputWidgetProperty("page", 10);
    cuttingMoveSpeed->setInputWidgetProperty("minimum", 0.001);
    cuttingMoveSpeed->setInputWidgetProperty("maximum", 100000);

    ConfigItem* cuttingMoveAcc = group->addConfigItem(
        "cuttingMoveAcc",
        tr("[03] Cutting Move Acceleration(mm/s<sup>2</sup>)"),
        tr("Cutting Move Acceleration"),
        45,
        DT_INT
    );
    cuttingMoveAcc->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    cuttingMoveAcc->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    cuttingMoveAcc->setInputWidgetProperty("maximumLineEditWidth", 75);
    cuttingMoveAcc->setInputWidgetProperty("step", 0.001);
    cuttingMoveAcc->setInputWidgetProperty("page", 10);
    cuttingMoveAcc->setInputWidgetProperty("minimum", 0.001);
    cuttingMoveAcc->setInputWidgetProperty("maximum", 10000);

    ConfigItem* cuttingTurnSpeed = group->addConfigItem(
        "cuttingTurnSpeed",
        tr("[04] Cutting Turn Speed(mm/s)"),
        tr("Cutting turn speed"),
        15,
        DT_INT
    );
    cuttingTurnSpeed->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    cuttingTurnSpeed->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    cuttingTurnSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    cuttingTurnSpeed->setInputWidgetProperty("step", 0.001);
    cuttingTurnSpeed->setInputWidgetProperty("page", 10);
    cuttingTurnSpeed->setInputWidgetProperty("minimum", 1);
    cuttingTurnSpeed->setInputWidgetProperty("maximum", 1000);

    ConfigItem* cuttingTurnAcc = group->addConfigItem(
        "cuttingTurnAcc",
        tr("[05] Cutting Turn Acceleration(mm/s<sup>2</sup>)"),
        tr("Cutting turn acceleration"),
        45,
        DT_INT
    );
    cuttingTurnAcc->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    cuttingTurnAcc->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    cuttingTurnAcc->setInputWidgetProperty("maximumLineEditWidth", 75);
    cuttingTurnAcc->setInputWidgetProperty("step", 0.001);
    cuttingTurnAcc->setInputWidgetProperty("page", 10);
    cuttingTurnAcc->setInputWidgetProperty("minimum", 1);
    cuttingTurnAcc->setInputWidgetProperty("maximum", 1000);

    ConfigItem* cuttingWorkAcc = group->addConfigItem(
        "cuttingWorkAcc",
        tr("[06] Cutting Work Acceleration(mm/s<sup>2</sup>)"),
        tr("Cutting Work acceleration"),
        60,
        DT_INT
    );
    cuttingWorkAcc->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    cuttingWorkAcc->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    cuttingWorkAcc->setInputWidgetProperty("maximumLineEditWidth", 75);
    cuttingWorkAcc->setInputWidgetProperty("step", 0.001);
    cuttingWorkAcc->setInputWidgetProperty("page", 10);
    cuttingWorkAcc->setInputWidgetProperty("minimum", 1);
    cuttingWorkAcc->setInputWidgetProperty("maximum", 10000);

    ConfigItem* cuttingMoveSpeedFactor = group->addConfigItem(
        "cuttingMoveSpeedFactor",
        tr("[07] Cutting Move Speed Factor"),
        tr("Cutting move speed factor"),
        2
    );
    cuttingMoveSpeedFactor->setInputWidgetProperty("maximumLineEditWidth", 75);
    cuttingMoveSpeedFactor->setInputWidgetProperty("minimum", 1);
    cuttingMoveSpeedFactor->setInputWidgetProperty("maximum", 100);

    ConfigItem* cuttingWorkSpeedFactor = group->addConfigItem(
        "cuttingWorkSpeedFactor",
        tr("[08] Cutting Work Speed Factor"),
        tr("Cutting Work speed factor"),
        2
    );
    cuttingWorkSpeedFactor->setInputWidgetProperty("maximumLineEditWidth", 75);
    cuttingWorkSpeedFactor->setInputWidgetProperty("minimum", 1);
    cuttingWorkSpeedFactor->setInputWidgetProperty("maximum", 100);

    ConfigItem* cuttingSpotSize = group->addConfigItem(
        "cuttingSpotSize",
        tr("[09] Cutting Spot Size"),
        tr("Cutting spot size"),
        1000,
        DT_INT
    );
    cuttingSpotSize->setInputWidgetProperty("maximumLineEditWidth", 75);
    cuttingSpotSize->setInputWidgetProperty("minimum", 1);
    cuttingSpotSize->setInputWidgetProperty("maximum", 1000);

    ConfigItem* scanXStartSpeed = group->addConfigItem(
        "scanXStartSpeed",
        tr("[10] Scan X Start Speed(mm/s)"),
        tr("Scan x start speed"),
        15,
        DT_INT
    );
    scanXStartSpeed->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    scanXStartSpeed->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    scanXStartSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    scanXStartSpeed->setInputWidgetProperty("step", 0.001);
    scanXStartSpeed->setInputWidgetProperty("page", 10);
    scanXStartSpeed->setInputWidgetProperty("minimum", 1);
    scanXStartSpeed->setInputWidgetProperty("maximum", 10000);

    ConfigItem* scanYStartSpeed = group->addConfigItem(
        "scanYStartSpeed",
        tr("[11] Scan Y Start Speed(mm/s)"),
        tr("Scan y start speed"),
        15,
        DT_INT
    );
    scanYStartSpeed->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    scanYStartSpeed->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    scanYStartSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    scanYStartSpeed->setInputWidgetProperty("step", 0.001);
    scanYStartSpeed->setInputWidgetProperty("page", 10);
    scanYStartSpeed->setInputWidgetProperty("minimum", 1);
    scanYStartSpeed->setInputWidgetProperty("maximum", 10000);

    ConfigItem* scanXAcc = group->addConfigItem(
        "scanXAcc",
        tr("[12] Scan X Acceleration(mm/s<sup>2</sup>)"),
        tr("Scan x acceleration"),
        5,
        DT_INT
    );
    scanXAcc->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    scanXAcc->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    scanXAcc->setInputWidgetProperty("maximumLineEditWidth", 75);
    scanXAcc->setInputWidgetProperty("step", 0.001);
    scanXAcc->setInputWidgetProperty("page", 10);
    scanXAcc->setInputWidgetProperty("minimum", 1);
    scanXAcc->setInputWidgetProperty("maximum", 10000);

    ConfigItem* scanYAcc = group->addConfigItem(
        "scanYAcc",
        tr("[13] Scan Y Acceleration(mm/s<sup>2</sup>)"),
        tr("Scan y acceleration"),
        45,
        DT_INT
    );
    scanYAcc->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    scanYAcc->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    scanYAcc->setInputWidgetProperty("maximumLineEditWidth", 75);
    scanYAcc->setInputWidgetProperty("step", 0.001);
    scanYAcc->setInputWidgetProperty("page", 10);
    scanYAcc->setInputWidgetProperty("minimum", 1);
    scanYAcc->setInputWidgetProperty("maximum", 10000);

    ConfigItem* scanRowSpeed = group->addConfigItem(
        "scanRowSpeed",
        tr("[14] Scan Row Speed(mm/s)"),
        tr("Scan row speed"),
        15,
        DT_REAL
    );
    scanRowSpeed->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    scanRowSpeed->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    scanRowSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    scanRowSpeed->setInputWidgetProperty("step", 0.001);
    scanRowSpeed->setInputWidgetProperty("page", 10);
    scanRowSpeed->setInputWidgetProperty("minimum", 1);
    scanRowSpeed->setInputWidgetProperty("maximum", 10000);

    ConfigItem* scanRowInterval = group->addConfigItem(
        "scanRowInterval",
        tr("[15] Scan Row Interval(mm)"),
        tr("Scan row interval"),
        0.007,
        DT_REAL
    );
    scanRowInterval->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    scanRowInterval->setValueFromWidgetHook(
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

    ConfigItem* scanReturnError = group->addConfigItem(
        "scanReturnError",
        tr("[16] Scan Return Error(mm/s)"),
        tr("Scan return error"),
        1000,
        DT_INT
    );
    scanReturnError->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    scanReturnError->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    scanReturnError->setInputWidgetProperty("maximumLineEditWidth", 75);
    scanReturnError->setInputWidgetProperty("step", 0.001);
    scanReturnError->setInputWidgetProperty("page", 10);
    scanReturnError->setInputWidgetProperty("minimum", 1);
    scanReturnError->setInputWidgetProperty("maximum", 100);

    ConfigItem* scanLaserPower = group->addConfigItem(
        "scanLaserPower",
        tr("[17] Scan Laser Power"),
        tr("Scan laser power"),
        120,
        DT_INT
    );
    scanLaserPower->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 10.0);
        }
    );
    scanLaserPower->setValueFromWidgetHook(
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

    ConfigItem* scanXResetEnabled = group->addConfigItem(
        "scanXResetEnabled",
        tr("[18] Scan X Reset Enabled"),
        tr("Scan x reset enabled"),
        true,
        DT_BOOL
    );
    scanXResetEnabled->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() == 1);
        }
    );
    scanXResetEnabled->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toBool() ? 1 : 0);
        }
    );

    ConfigItem* scanYResetEnabled = group->addConfigItem(
        "scanYResetEnabled",
        tr("[19] Scan Y Reset Enabled"),
        tr("Scan y reset enabled"),
        true,
        DT_BOOL
    );
    scanYResetEnabled->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() == 1);
        }
    );
    scanYResetEnabled->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toBool() ? 1 : 0);
        }
    );

    ConfigItem* scanZResetEnabled = group->addConfigItem(
        "scanZResetEnabled",
        tr("[20] Scan Z Reset Enabled"),
        tr("Scan z reset enabled"),
        true,
        DT_BOOL
    );
    scanZResetEnabled->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() == 1);
        }
    );
    scanZResetEnabled->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toBool() ? 1 : 0);
        }
    );

    ConfigItem* resetSpeed = group->addConfigItem(
        "resetSpeed",
        tr("[21] Reset speed"),
        tr("Reset speed(mm/s)"),
        10,
        DT_INT
    );
    resetSpeed->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    resetSpeed->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    resetSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    resetSpeed->setInputWidgetProperty("step", 0.001);
    resetSpeed->setInputWidgetProperty("page", 10);
    resetSpeed->setInputWidgetProperty("minimum", 0.001);
    resetSpeed->setInputWidgetProperty("maximum", 10000);

    ConfigItem* scanReturnPos = group->addConfigItem(
        "scanReturnPos",
        tr("[22] Scan Return pos"),
        tr("Scan return pos"),
        0
    );
    scanReturnPos->setInputWidgetProperty("maximumLineEditWidth", 75);
    scanReturnPos->setInputWidgetProperty("page", 1000);
    scanReturnPos->setInputWidgetProperty("minimum", 0);
    scanReturnPos->setInputWidgetProperty("maximum", 100000);

    ConfigItem* backlashXInterval = group->addConfigItem(
        "backlashXInterval",
        tr("[23] Backlash X Interval(mm/s)"),
        tr("Backlash x interval"),
        0,
        DT_INT
    );
    backlashXInterval->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    backlashXInterval->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    backlashXInterval->setInputWidgetProperty("maximumLineEditWidth", 75);
    backlashXInterval->setInputWidgetProperty("step", 0.001);
    backlashXInterval->setInputWidgetProperty("page", 10);
    backlashXInterval->setInputWidgetProperty("minimum", 0);
    backlashXInterval->setInputWidgetProperty("maximum", 100);

    ConfigItem* backlashYInterval = group->addConfigItem(
        "backlashYInterval",
        tr("[24] Backlash Y Interval(mm/s)"),
        tr("Backlash y interval"),
        0,
        DT_INT
    );
    backlashYInterval->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    backlashYInterval->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    backlashYInterval->setInputWidgetProperty("maximumLineEditWidth", 75);
    backlashYInterval->setInputWidgetProperty("step", 0.001);
    backlashYInterval->setInputWidgetProperty("page", 10);
    backlashYInterval->setInputWidgetProperty("minimum", 0);
    backlashYInterval->setInputWidgetProperty("maximum", 100);

    ConfigItem* backlashZInterval = group->addConfigItem(
        "backlashZInterval",
        tr("[25] Backlash Z Interval(mm/s)"),
        tr("Backlash z interval"),
        0,
        DT_INT
    );
    backlashZInterval->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    backlashZInterval->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    backlashZInterval->setInputWidgetProperty("maximumLineEditWidth", 75);
    backlashZInterval->setInputWidgetProperty("step", 0.001);
    backlashZInterval->setInputWidgetProperty("page", 10);
    backlashZInterval->setInputWidgetProperty("minimum", 0);
    backlashZInterval->setInputWidgetProperty("maximum", 100);

    ConfigItem* defaultRunSpeed = group->addConfigItem(
        "defaultRunSpeed",
        tr("[26] Default running speed(mm/s)"),
        tr("Default running speed(mm/s)"),
        10000,
        DT_INT
    );
    defaultRunSpeed->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    defaultRunSpeed->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    defaultRunSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    defaultRunSpeed->setInputWidgetProperty("step", 0.001);
    defaultRunSpeed->setInputWidgetProperty("page", 10);
    defaultRunSpeed->setInputWidgetProperty("minimum", 1);
    defaultRunSpeed->setInputWidgetProperty("maximum", 10000);

    ConfigItem* defaultMaxCuttingPower = group->addConfigItem(
        "defaultMaxCuttingPower",
        tr("[27] Default max cutting power"),
        tr("Default max cutting power"),
        1000,
        DT_INT
    );
    defaultMaxCuttingPower->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 10.0);
        }
    );
    defaultMaxCuttingPower->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 10));
        }
    );
    defaultMaxCuttingPower->setInputWidgetProperty("textTemplate", "%1%");
    defaultMaxCuttingPower->setInputWidgetProperty("maximumLineEditWidth", 75);
    defaultMaxCuttingPower->setInputWidgetProperty("step", 0.1);
    defaultMaxCuttingPower->setInputWidgetProperty("page", 10);
    defaultMaxCuttingPower->setInputWidgetProperty("minimum", 0);
    defaultMaxCuttingPower->setInputWidgetProperty("maximum", 100);

    ConfigItem* defaultMinCuttingPower = group->addConfigItem(
        "defaultMinCuttingPower",
        tr("[28] Default min cutting power"),
        tr("Default min cutting power"),
        100,
        DT_INT
    );
    defaultMinCuttingPower->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 10.0);
        }
    );
    defaultMinCuttingPower->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 10));
        }
    );
    defaultMinCuttingPower->setInputWidgetProperty("textTemplate", "%1%");
    defaultMinCuttingPower->setInputWidgetProperty("maximumLineEditWidth", 75);
    defaultMinCuttingPower->setInputWidgetProperty("step", 0.1);
    defaultMinCuttingPower->setInputWidgetProperty("page", 10);
    defaultMinCuttingPower->setInputWidgetProperty("minimum", 0);
    defaultMinCuttingPower->setInputWidgetProperty("maximum", 100);

    ConfigItem* defaultScanSpeed = group->addConfigItem(
        "defaultScanSpeed",
        tr("[29] Default scan speed(mm/s)"),
        tr("Default scan speed(mm/s)"),
        500000,
        DT_INT
    );
    defaultScanSpeed->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    defaultScanSpeed->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    defaultScanSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    defaultScanSpeed->setInputWidgetProperty("step", 0.001);
    defaultScanSpeed->setInputWidgetProperty("page", 10);
    defaultScanSpeed->setInputWidgetProperty("minimum", 0.001);
    defaultScanSpeed->setInputWidgetProperty("maximum", 10000);

    ConfigItem* maxScanGrayRatio = group->addConfigItem(
        "maxScanGrayRatio",
        tr("[30] Max scan gray ratio"),
        tr("Max scan gray ratio"),
        800,
        DT_INT
    );
    maxScanGrayRatio->setInputWidgetProperty("maximumLineEditWidth", 75);
    maxScanGrayRatio->setInputWidgetProperty("minimum", 1);
    maxScanGrayRatio->setInputWidgetProperty("maximum", 2000);

    ConfigItem* minScanGrayRatio = group->addConfigItem(
        "minScanGrayRatio",
        tr("[31] Min scan gray ratio"),
        tr("Min scan gray ratio"),
        50,
        DT_INT
    );
    minScanGrayRatio->setInputWidgetProperty("maximumLineEditWidth", 75);
    minScanGrayRatio->setInputWidgetProperty("minimum", 1);
    minScanGrayRatio->setInputWidgetProperty("maximum", 2000);
}

void Config::loadSystemRegisters()
{
    ConfigItemGroup* group = new Config::SystemRegister;
    Config::SystemRegister::group = group;

    ConfigItem* head = group->addConfigItem(
        "head",
        tr("[00] Head Data"),
        tr("Head data for testing"),
        0x12345678,
        DT_INT
    );
    head->setReadOnly();
    head->setInputWidgetType(IWT_LineEdit);
    head->setInputWidgetProperty("readOnly", true);

    ConfigItem* password = group->addConfigItem(
        "password",
        tr("[01] Password"),
        tr("Password"),
        "",
        DT_STRING
    );
    password->setWriteOnly();
    password->setInputWidgetType(IWT_LineEdit);
    password->setVisible(false);

    ConfigItem* storedPassword = group->addConfigItem(
        "storedPassword",
        tr("[02] Stored Password"),
        tr("Stored password"),
        "",
        DT_STRING
    );
    storedPassword->setWriteOnly();
    storedPassword->setInputWidgetType(IWT_LineEdit);
    storedPassword->setVisible(false);

    ConfigItem* hardwareID1 = group->addConfigItem(
        "hardwareID1",
        tr("[03] Hardware ID1"),
        tr("Hardware ID1"),
        "",
        DT_STRING
    );
    hardwareID1->setReadOnly();
    hardwareID1->setInputWidgetType(IWT_LineEdit);
    hardwareID1->setInputWidgetProperty("readOnly", true);

    ConfigItem* hardwareID2 = group->addConfigItem(
        "hardwareID2",
        tr("[04] Hardware ID2"),
        tr("Hardware ID2"),
        "",
        DT_STRING
    );
    hardwareID2->setReadOnly();
    hardwareID2->setInputWidgetType(IWT_LineEdit);
    hardwareID2->setInputWidgetProperty("readOnly", true);

    ConfigItem* hardwareID3 = group->addConfigItem(
        "hardwareID3",
        tr("[05] Hardware ID3"),
        tr("Hardware ID3"),
        "",
        DT_STRING
    );
    hardwareID3->setReadOnly();
    hardwareID3->setInputWidgetType(IWT_LineEdit);
    hardwareID3->setInputWidgetProperty("readOnly", true);

    ConfigItem* cdKey1 = group->addConfigItem(
        "cdKey1",
        tr("[06] cdKey1"),
        tr("cdKey1"),
        "",
        DT_STRING
    );
    cdKey1->setInputWidgetType(IWT_LineEdit);

    ConfigItem* cdKey2 = group->addConfigItem(
        "cdKey2",
        tr("[07] cdKey2"),
        tr("cdKey2"),
        "",
        DT_STRING
    );
    cdKey2->setInputWidgetType(IWT_LineEdit);

    ConfigItem* cdKey3 = group->addConfigItem(
        "cdKey3",
        tr("[08] cdKey3"),
        tr("cdKey3"),
        "",
        DT_STRING
    );
    cdKey3->setInputWidgetType(IWT_LineEdit);

    ConfigItem* sysRunTime = group->addConfigItem(
        "sysRunTime",
        tr("[09] System Run Time"),
        tr("System run time"),
        0,
        DT_INT
    );
    sysRunTime->setReadOnly(true);
    sysRunTime->setInputWidgetType(IWT_LineEdit);

    ConfigItem* laserRunTime = group->addConfigItem(
        "laserRunTime",
        tr("[10] Laser Run Time"),
        tr("Laser run time"),
        0,
        DT_INT
    );
    laserRunTime->setReadOnly(true);
    laserRunTime->setInputWidgetType(IWT_LineEdit);

    ConfigItem* sysRunNum = group->addConfigItem(
        "sysRunNum",
        tr("[11] System Run Times"),
        tr("System run times"),
        0,
        DT_INT
    );
    sysRunNum->setReadOnly(true);
    sysRunNum->setInputWidgetType(IWT_LineEdit);

    ConfigItem* xMaxLength = group->addConfigItem(
        "xMaxLength",
        tr("[12] X Max Length(mm)"),
        tr("X max length"),
        320000,
        DT_INT
    );
    xMaxLength->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    xMaxLength->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    xMaxLength->setInputWidgetProperty("maximumLineEditWidth", 75);
    xMaxLength->setInputWidgetProperty("step", 0.001);
    xMaxLength->setInputWidgetProperty("page", 10);
    xMaxLength->setInputWidgetProperty("minimum", 1);
    xMaxLength->setInputWidgetProperty("maximum", 5000);
    xMaxLength->setInputWidgetProperty("decimals", 3);

    ConfigItem* xDirPhase = group->addConfigItem(
        "xDirPhase",
        tr("[13] X Dir Phase"),
        tr("X dir phase"),
        1,
        DT_INT
    );
    xDirPhase->setInputWidgetType(IWT_ComboBox);
    xDirPhase->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item, InputWidgetWrapper* wrapper)
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

    ConfigItem* xLimitPhase = group->addConfigItem(
        "xLimitPhase",
        tr("[14] X Limit Phase"),
        tr("X limit phase"),
        0
    );
    xLimitPhase->setInputWidgetType(IWT_ComboBox);
    xLimitPhase->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item, InputWidgetWrapper* wrapper)
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

    ConfigItem* xZeroDev = group->addConfigItem(
        "xZeroDev",
        tr("[15] X Zero Dev(mm)"),
        tr("X zero dev"),
        2000,
        DT_INT
    );
    xZeroDev->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    xZeroDev->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    xZeroDev->setInputWidgetProperty("maximumLineEditWidth", 75);
    xZeroDev->setInputWidgetProperty("step", 0.001);
    xZeroDev->setInputWidgetProperty("page", 1);
    xZeroDev->setInputWidgetProperty("minimum", 0);
    xZeroDev->setInputWidgetProperty("maximum", 10);
    xZeroDev->setInputWidgetProperty("decimals", 3);

    ConfigItem* xStepLength = group->addConfigItem(
        "xStepLength",
        tr("[16] X Step Length(mm)"),
        tr("X step length"),
        3164557,
        DT_INT
    );
    xStepLength->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000000));
        }
    );
    xStepLength->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000000.0);
        }
    );
    xStepLength->setInputWidgetProperty("maximumLineEditWidth", 75);
    xStepLength->setInputWidgetProperty("step", 0.000001);
    xStepLength->setInputWidgetProperty("page", 0.001);
    xStepLength->setInputWidgetProperty("minimum", 0.000001);
    xStepLength->setInputWidgetProperty("maximum", 100);
    xStepLength->setInputWidgetProperty("decimals", 6);

    ConfigItem* xLimitNum = group->addConfigItem(
        "xLimitNum",
        tr("[17] X Limit number"),
        tr("X limit number"),
        0
    );
    xLimitNum->setInputWidgetType(IWT_ComboBox);
    xLimitNum->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item, InputWidgetWrapper* wrapper)
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

    ConfigItem* xResetEnabled = group->addConfigItem(
        "xResetEnabled",
        tr("[18] X Reset Enabled"),
        tr("X reset enabled"),
        true,
        DT_BOOL
    );

    ConfigItem* xMotorNum = group->addConfigItem(
        "xMotorNum",
        tr("[19] X Motor number"),
        tr("X motor number"),
        0,
        DT_INT
    );
    xMotorNum->setInputWidgetType(IWT_ComboBox);
    xMotorNum->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item, InputWidgetWrapper* wrapper)
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

    ConfigItem* xMotorCurrent = group->addConfigItem(
        "xMotorCurrent",
        tr("[20] X Motor current"),
        tr("X motor current"),
        500,
        DT_INT
    );
    xMotorCurrent->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 10));
        }
    );
    xMotorCurrent->setValueToWidgetHook(
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

    ConfigItem* xStartSpeed = group->addConfigItem(
        "xStartSpeed",
        tr("[21] X Start speed(mm/s)"),
        tr("X start speed"),
        15000,
        DT_INT
    );
    xStartSpeed->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    xStartSpeed->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    xStartSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    xStartSpeed->setInputWidgetProperty("step", 0.001);
    xStartSpeed->setInputWidgetProperty("page", 10);
    xStartSpeed->setInputWidgetProperty("minimum", 0.001);
    xStartSpeed->setInputWidgetProperty("maximum", 1000);
    xStartSpeed->setInputWidgetProperty("decimals", 3);

    ConfigItem* xMaxSpeed = group->addConfigItem(
        "xMaxSpeed",
        tr("[22] X Max speed(mm/s)"),
        tr("X max speed"),
        4500,
        DT_INT
    );
    xMaxSpeed->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    xMaxSpeed->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    xMaxSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    xMaxSpeed->setInputWidgetProperty("step", 0.001);
    xMaxSpeed->setInputWidgetProperty("page", 10);
    xMaxSpeed->setInputWidgetProperty("minimum", 0.001);
    xMaxSpeed->setInputWidgetProperty("maximum", 1000);
    xMaxSpeed->setInputWidgetProperty("decimals", 3);

    ConfigItem* xMaxAcceleration = group->addConfigItem(
        "xMaxAcceleration",
        tr("[23] X Max Acceleration(mm/s<sup>2</sup>)"),
        tr("X max acceleration"),
        45,
        DT_INT
    );
    xMaxAcceleration->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    xMaxAcceleration->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    xMaxAcceleration->setInputWidgetProperty("maximumLineEditWidth", 75);
    xMaxAcceleration->setInputWidgetProperty("step", 0.001);
    xMaxAcceleration->setInputWidgetProperty("page", 10);
    xMaxAcceleration->setInputWidgetProperty("minimum", 1);
    xMaxAcceleration->setInputWidgetProperty("maximum", 1000);
    xMaxAcceleration->setInputWidgetProperty("decimals", 3);

    ConfigItem* xUrgentAcceleration = group->addConfigItem(
        "xUrgentAcceleration",
        tr("[24] X Urgent Acceleration(mm/s<sup>2</sup>)"),
        tr("X urgent acceleration"),
        45,
        DT_INT
    );
    xUrgentAcceleration->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    xUrgentAcceleration->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    xUrgentAcceleration->setInputWidgetProperty("maximumLineEditWidth", 75);
    xUrgentAcceleration->setInputWidgetProperty("step", 0.001);
    xUrgentAcceleration->setInputWidgetProperty("page", 10);
    xUrgentAcceleration->setInputWidgetProperty("minimum", 1);
    xUrgentAcceleration->setInputWidgetProperty("maximum", 10000);
    xUrgentAcceleration->setInputWidgetProperty("decimals", 3);

    ConfigItem* yMaxLength = group->addConfigItem(
        "yMaxLength",
        tr("[25] Y Max Length(mm)"),
        tr("Y max length"),
        200000,
        DT_INT
    );
    yMaxLength->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    yMaxLength->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    yMaxLength->setInputWidgetProperty("maximumLineEditWidth", 75);
    yMaxLength->setInputWidgetProperty("step", 0.001);
    yMaxLength->setInputWidgetProperty("page", 1);
    yMaxLength->setInputWidgetProperty("minimum", 1);
    yMaxLength->setInputWidgetProperty("maximum", 5000);
    yMaxLength->setInputWidgetProperty("decimals", 3);

    ConfigItem* yDirPhase = group->addConfigItem(
        "yDirPhase",
        tr("[26] Y Dir Phase"),
        tr("Y dir phase"),
        1,
        DT_INT
    );
    yDirPhase->setInputWidgetType(IWT_ComboBox);
    yDirPhase->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item, InputWidgetWrapper* wrapper)
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

    ConfigItem* yLimitPhase = group->addConfigItem(
        "yLimitPhase",
        tr("[27] Y Limit Phase"),
        tr("Y limit phase"),
        0,
        DT_INT
    );
    yLimitPhase->setInputWidgetType(IWT_ComboBox);
    yLimitPhase->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item, InputWidgetWrapper* wrapper)
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

    ConfigItem* yZeroDev = group->addConfigItem(
        "yZeroDev",
        tr("[28] Y Zero Dev(mm)"),
        tr("Y zero dev"),
        2000,
        DT_INT
    );
    yZeroDev->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    yZeroDev->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    yZeroDev->setInputWidgetProperty("maximumLineEditWidth", 75);
    yZeroDev->setInputWidgetProperty("step", 0.001);
    yZeroDev->setInputWidgetProperty("page", 1);
    yZeroDev->setInputWidgetProperty("minimum", 1);
    yZeroDev->setInputWidgetProperty("maximum", 10);
    yZeroDev->setInputWidgetProperty("decimals", 3);

    ConfigItem* yStepLength = group->addConfigItem(
        "yStepLength",
        tr("[29] Y Step Length(mm)"),
        tr("Y step length"),
        3164557,
        DT_INT
    );
    yStepLength->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000000));
        }
    );
    yStepLength->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000000.0);
        }
    );
    yStepLength->setInputWidgetProperty("maximumLineEditWidth", 75);
    yStepLength->setInputWidgetProperty("step", 0.000001);
    yStepLength->setInputWidgetProperty("page", 0.001);
    yStepLength->setInputWidgetProperty("minimum", 0.000001);
    yStepLength->setInputWidgetProperty("maximum", 100);
    yStepLength->setInputWidgetProperty("decimals", 6);

    ConfigItem* yLimitNum = group->addConfigItem(
        "yLimitNum",
        tr("[30] Y Limit number"),
        tr("Y limit number"),
        0,
        DT_INT
    );
    yLimitNum->setInputWidgetType(IWT_ComboBox);
    yLimitNum->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item, InputWidgetWrapper* wrapper)
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

    ConfigItem* yResetEnabled = group->addConfigItem(
        "yResetEnabled",
        tr("[31] Y Reset Enabled"),
        tr("Y reset enabled"),
        true,
        DT_BOOL
    );

    ConfigItem* yMotorNum = group->addConfigItem(
        "yMotorNum",
        tr("[32] Y Motor number"),
        tr("Y motor number"),
        0,
        DT_INT
    );
    yMotorNum->setInputWidgetType(IWT_ComboBox);
    yMotorNum->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item, InputWidgetWrapper* wrapper)
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

    ConfigItem* yMotorCurrent = group->addConfigItem(
        "yMotorCurrent",
        tr("[33] Y Motor current"),
        tr("Y motor current"),
        500,
        DT_INT
    );
    yMotorCurrent->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 10));
        }
    );
    yMotorCurrent->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 10.0);
        }
    );
    yMotorCurrent->setInputWidgetProperty("textTemplate", "%1%");
    yMotorCurrent->setInputWidgetProperty("maximumLineEditWidth", 75);
    yMotorCurrent->setInputWidgetProperty("step", 0.1);
    yMotorCurrent->setInputWidgetProperty("page", 10);
    yMotorCurrent->setInputWidgetProperty("minimum", 1);
    yMotorCurrent->setInputWidgetProperty("maximum", 100);

    ConfigItem* yStartSpeed = group->addConfigItem(
        "yStartSpeed",
        tr("[34] Y Start speed(mm/s)"),
        tr("Y start speed"),
        15,
        DT_INT
    );
    yStartSpeed->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    yStartSpeed->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    yStartSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    yStartSpeed->setInputWidgetProperty("step", 0.001);
    yStartSpeed->setInputWidgetProperty("page", 10);
    yStartSpeed->setInputWidgetProperty("minimum", 1);
    yStartSpeed->setInputWidgetProperty("maximum", 100);
    yStartSpeed->setInputWidgetProperty("decimals", 3);

    ConfigItem* yMaxSpeed = group->addConfigItem(
        "yMaxSpeed",
        tr("[35] Y Max speed(mm/s)"),
        tr("Y max speed"),
        45,
        DT_REAL
    );
    yMaxSpeed->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    yMaxSpeed->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    yMaxSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    yMaxSpeed->setInputWidgetProperty("step", 0.001);
    yMaxSpeed->setInputWidgetProperty("page", 10);
    yMaxSpeed->setInputWidgetProperty("minimum", 1);
    yMaxSpeed->setInputWidgetProperty("maximum", 1000);
    yMaxSpeed->setInputWidgetProperty("decimals", 3);

    ConfigItem* yMaxAcceleration = group->addConfigItem(
        "yMaxAcceleration",
        tr("[36] Y Max Acceleration(mm/s<sup>2</sup>)"),
        tr("Y max acceleration"),
        45,
        DT_REAL
    );
    yMaxAcceleration->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    yMaxAcceleration->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    yMaxAcceleration->setInputWidgetProperty("maximumLineEditWidth", 75);
    yMaxAcceleration->setInputWidgetProperty("step", 0.001);
    yMaxAcceleration->setInputWidgetProperty("page", 10);
    yMaxAcceleration->setInputWidgetProperty("minimum", 1);
    yMaxAcceleration->setInputWidgetProperty("maximum", 1000);
    yMaxAcceleration->setInputWidgetProperty("decimals", 3);

    ConfigItem* yUrgentAcceleration = group->addConfigItem(
        "yUrgentAcceleration",
        tr("[37] Y Urgent Acceleration(mm/s<sup>2</sup>)"),
        tr("Y urgent acceleration"),
        20,
        DT_REAL
    );
    yUrgentAcceleration->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    yUrgentAcceleration->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    yUrgentAcceleration->setInputWidgetProperty("maximumLineEditWidth", 75);
    yUrgentAcceleration->setInputWidgetProperty("step", 0.001);
    yUrgentAcceleration->setInputWidgetProperty("page", 10);
    yUrgentAcceleration->setInputWidgetProperty("minimum", 1);
    yUrgentAcceleration->setInputWidgetProperty("maximum", 10000);
    yUrgentAcceleration->setInputWidgetProperty("decimals", 3);

    ConfigItem* zMaxLength = group->addConfigItem(
        "zMaxLength",
        tr("[38] Z Max Length(mm)"),
        tr("Z max length"),
        200000,
        DT_INT
    );
    zMaxLength->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    zMaxLength->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    zMaxLength->setInputWidgetProperty("maximumLineEditWidth", 75);
    zMaxLength->setInputWidgetProperty("step", 0.001);
    zMaxLength->setInputWidgetProperty("page", 1);
    zMaxLength->setInputWidgetProperty("minimum", 1);
    zMaxLength->setInputWidgetProperty("maximum", 5000);
    zMaxLength->setInputWidgetProperty("decimals", 3);

    ConfigItem* zDirPhase = group->addConfigItem(
        "zDirPhase",
        tr("[39] Z Dir Phase"),
        tr("Z dir phase"),
        1,
        DT_INT
    );
    zDirPhase->setInputWidgetType(IWT_ComboBox);
    zDirPhase->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item, InputWidgetWrapper* wrapper)
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

    ConfigItem* zLimitPhase = group->addConfigItem(
        "zLimitPhase",
        tr("[40] Z Limit Length"),
        tr("Z limit length"),
        0,
        DT_INT
    );
    zLimitPhase->setInputWidgetType(IWT_ComboBox);
    zLimitPhase->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item, InputWidgetWrapper* wrapper)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (!comboBox)
                return;

            comboBox->blockSignals(true);
            comboBox->addItem(tr("0"), 0);
            comboBox->addItem(tr("1"), 1);
            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
            comboBox->blockSignals(false);
        }
    );

    ConfigItem* zZeroDev = group->addConfigItem(
        "zZeroDev",
        tr("[41] Z Zero Dev(mm)"),
        tr("Z zero dev"),
        2000,
        DT_INT
    );
    zZeroDev->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    zZeroDev->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    zZeroDev->setInputWidgetProperty("maximumLineEditWidth", 75);
    zZeroDev->setInputWidgetProperty("step", 0.001);
    zZeroDev->setInputWidgetProperty("page", 1);
    zZeroDev->setInputWidgetProperty("minimum", 1);
    zZeroDev->setInputWidgetProperty("maximum", 10);
    zZeroDev->setInputWidgetProperty("decimals", 3);

    ConfigItem* zStepLength = group->addConfigItem(
        "zStepLength",
        tr("[42] Z Step Length(mm)"),
        tr("Z step length"),
        6200000,
        DT_INT
    );
    zStepLength->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000000));
        }
    );
    zStepLength->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000000.0);
        }
    );
    zStepLength->setInputWidgetProperty("maximumLineEditWidth", 75);
    zStepLength->setInputWidgetProperty("step", 0.000001);
    zStepLength->setInputWidgetProperty("page", 0.001);
    zStepLength->setInputWidgetProperty("minimum", 0.000001);
    zStepLength->setInputWidgetProperty("maximum", 100);
    zStepLength->setInputWidgetProperty("decimals", 6);

    ConfigItem* zLimitNum = group->addConfigItem(
        "zLimitNum",
        tr("[43] Z Limit number"),
        tr("Z limit number"),
        0,
        DT_INT
    );
    zLimitNum->setInputWidgetType(IWT_ComboBox);
    zLimitNum->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item, InputWidgetWrapper* wrapper)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (!comboBox)
                return;

            comboBox->addItem(tr("0"), 0);
            comboBox->addItem(tr("1"), 1);
            comboBox->addItem(tr("2"), 2);
            comboBox->addItem(tr("3"), 3);
            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );

    ConfigItem* zResetEnabled = group->addConfigItem(
        "zResetEnabled",
        tr("[44] Z Reset Enabled"),
        tr("Z reset enabled"),
        true,
        DT_BOOL
    );

    ConfigItem* zMotorNum = group->addConfigItem(
        "zMotorNum",
        tr("[45] Z Motor number"),
        tr("Z motor number"),
        0,
        DT_INT
    );
    zMotorNum->setInputWidgetType(IWT_ComboBox);
    zMotorNum->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item, InputWidgetWrapper* wrapper)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (!comboBox)
                return;

            comboBox->addItem(tr("0"), 0);
            comboBox->addItem(tr("1"), 1);
            comboBox->addItem(tr("2"), 2);
            comboBox->addItem(tr("3"), 3);
            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );

    ConfigItem* zMotorCurrent = group->addConfigItem(
        "zMotorCurrent",
        tr("[46] Z Motor current"),
        tr("Z motor current"),
        500,
        DT_INT
    );
    zMotorCurrent->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 10));
        }
    );
    zMotorCurrent->setValueToWidgetHook(
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

    ConfigItem* zStartSpeed = group->addConfigItem(
        "zStartSpeed",
        tr("[47] Z Start speed(mm/s)"),
        tr("Z start speed"),
        15000,
        DT_INT
    );
    zStartSpeed->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    zStartSpeed->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    zStartSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    zStartSpeed->setInputWidgetProperty("step", 0.001);
    zStartSpeed->setInputWidgetProperty("page", 10);
    zStartSpeed->setInputWidgetProperty("minimum", 1);
    zStartSpeed->setInputWidgetProperty("maximum", 100);
    zStartSpeed->setInputWidgetProperty("decimals", 3);

    ConfigItem* zMaxSpeed = group->addConfigItem(
        "zMaxSpeed",
        tr("[48] Z Max speed(mm/s)"),
        tr("Z max speed"),
        10000,
        DT_INT
    );
    zMaxSpeed->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    zMaxSpeed->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    zMaxSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    zMaxSpeed->setInputWidgetProperty("step", 0.001);
    zMaxSpeed->setInputWidgetProperty("page", 10);
    zMaxSpeed->setInputWidgetProperty("minimum", 1);
    zMaxSpeed->setInputWidgetProperty("maximum", 1000);
    zMaxSpeed->setInputWidgetProperty("decimals", 3);

    ConfigItem* zMaxAcceleration = group->addConfigItem(
        "zMaxAcceleration",
        tr("[49] Z Max Acceleration(mm/s<sup>2</sup>)"),
        tr("Z max acceleration"),
        30000,
        DT_INT
    );
    zMaxAcceleration->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    zMaxAcceleration->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    zMaxAcceleration->setInputWidgetProperty("maximumLineEditWidth", 75);
    zMaxAcceleration->setInputWidgetProperty("step", 0.001);
    zMaxAcceleration->setInputWidgetProperty("page", 10);
    zMaxAcceleration->setInputWidgetProperty("minimum", 1);
    zMaxAcceleration->setInputWidgetProperty("maximum", 1000);
    zMaxAcceleration->setInputWidgetProperty("decimals", 3);

    ConfigItem* zUrgentAcceleration = group->addConfigItem(
        "zUrgentAcceleration",
        tr("[50] Z Urgent Acceleration(mm/s<sup>2</sup>)"),
        tr("Z urgent acceleration"),
        30000,
        DT_INT
    );
    zUrgentAcceleration->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 1000));
        }
    );
    zUrgentAcceleration->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 1000.0);
        }
    );
    zUrgentAcceleration->setInputWidgetProperty("maximumLineEditWidth", 75);
    zUrgentAcceleration->setInputWidgetProperty("step", 0.001);
    zUrgentAcceleration->setInputWidgetProperty("page", 10);
    zUrgentAcceleration->setInputWidgetProperty("minimum", 1);
    zUrgentAcceleration->setInputWidgetProperty("maximum", 10000);
    zUrgentAcceleration->setInputWidgetProperty("decimals", 3);

    ConfigItem* laserMaxPower = group->addConfigItem(
        "laserMaxPower",
        tr("[51] Laser Max Power"),
        tr("Laser max power"),
        1000,
        DT_INT
    );
    laserMaxPower->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 10));
        }
    );
    laserMaxPower->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 10.0);
        }
    );
    laserMaxPower->setInputWidgetProperty("textTemplate", "%1%");
    laserMaxPower->setInputWidgetProperty("maximumLineEditWidth", 75);
    laserMaxPower->setInputWidgetProperty("minimum", 1);
    laserMaxPower->setInputWidgetProperty("maximum", 100);
    laserMaxPower->setStoreStrategy(SS_DIRECTLY);

    ConfigItem* laserMinPower = group->addConfigItem(
        "laserMinPower",
        tr("[52] Laser Min Power"),
        tr("Laser min power"),
        100,
        DT_INT
    );
    laserMinPower->setValueFromWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(qRound(value.toReal() * 10));
        }
    );
    laserMinPower->setValueToWidgetHook(
        [](const QVariant& value)
        {
            return QVariant(value.toInt() / 10.0);
        }
    );
    laserMinPower->setInputWidgetProperty("textTemplate", "%1%");
    laserMinPower->setInputWidgetProperty("maximumLineEditWidth", 75);
    laserMinPower->setInputWidgetProperty("minimum", 1);
    laserMinPower->setInputWidgetProperty("maximum", 100);
    laserMinPower->setStoreStrategy(SS_DIRECTLY);

    ConfigItem* laserPowerFreq = group->addConfigItem(
        "laserPowerFreq",
        tr("[53] Laser Min Power"),
        tr("Laser min power"),
        4000
    );
    laserPowerFreq->setInputWidgetProperty("maximumLineEditWidth", 75);
    laserPowerFreq->setInputWidgetProperty("page", 1000);
    laserPowerFreq->setInputWidgetProperty("minimum", 1);
    laserPowerFreq->setInputWidgetProperty("maximum", 10000);
    laserPowerFreq->setStoreStrategy(SS_DIRECTLY);

    ConfigItem* xPhaseEnabled = group->addConfigItem(
        "xPhaseEnabled",
        tr("[54] X Phase Enabled"),
        tr("X phase enabled"),
        true,
        DT_BOOL
    );

    ConfigItem* yPhaseEnabled = group->addConfigItem(
        "yPhaseEnabled",
        tr("[55] Y Phase Enabled"),
        tr("Y phase enabled"),
        true,
        DT_BOOL
    );

    ConfigItem* zPhaseEnabled = group->addConfigItem(
        "zPhaseEnabled",
        tr("[56] Z Phase Enabled"),
        tr("Z phase enabled"),
        true,
        DT_BOOL
    );

    ConfigItem* deviceOrigin = group->addConfigItem(
        "deviceOrigin",
        tr("[57] Device Origin"),
        tr("[57] Device Origin"),
        0,
        DT_INT
    );
    deviceOrigin->setInputWidgetType(IWT_Custom);
    deviceOrigin->setCreateWidgetHook(
        [=](ConfigItem* item) {
            RadioButtonGroup* widget = new RadioButtonGroup(2, 2);
            widget->setValues(QList<int>() << 0 << 1 << 3 << 2);
            return qobject_cast<QWidget*>(widget);
        }
    );
    deviceOrigin->setWidgetInitializeHook(
        [=](QWidget* widget, ConfigItem* item, InputWidgetWrapper* wrapper) {
            RadioButtonGroup* radioGroup = qobject_cast<RadioButtonGroup*>(widget);
            QObject::connect(radioGroup, &RadioButtonGroup::valueChanged, wrapper, QOverload<int>::of(&InputWidgetWrapper::onValueChanged));
        }
    );
    deviceOrigin->setUpdateWidgetValueHook(
        [=](QWidget* widget, const QVariant& value) {
            RadioButtonGroup* radioGroup = qobject_cast<RadioButtonGroup*>(widget);
            radioGroup->setValue(value.toInt());
        }
    );
}

void Config::loadDebug()
{
    ConfigItemGroup* group = new Config::Debug;
    Config::Debug::group = group;

    ConfigItem* showPrimitiveName = group->addConfigItem(
        "showPrimitiveName",
        tr("Show Primitive Name"),
        tr("Show primitve name."),
        false,
        DT_BOOL
    );

    ConfigItem* showPrimitiveFirstPoint = group->addConfigItem(
        "showPrimitiveFirstPoint",
        tr("Show Primitive First Point"),
        tr("Show primitve first point."),
        false,
        DT_BOOL
    );

    ConfigItem* generatePathImage = group->addConfigItem(
        "generatePathImage",
        tr("Generate Path Image"),
        tr("Generate path image."),
        false,
        DT_BOOL
    );

    ConfigItem* generateMachiningImage = group->addConfigItem(
        "generateMachiningImage",
        tr("Generate Machining Image"),
        tr("Generate machining image."),
        false,
        DT_BOOL
    );

    ConfigItem* enableOptimizeInteraction = group->addConfigItem(
        "enableOptimizeInteraction",
        tr("Enable Optimize Interaction"),
        tr("Enable Optimize Interaction"),
        false,
        DT_BOOL
    );
}

QList<ConfigItemGroup*> Config::getGroups()
{
    return groups;
}

void Config::refreshTranslation()
{
}

void Config::destroy()
{
    save();
    qDeleteAll(groups);
    groups.clear();
    groupsMap.clear();
}
