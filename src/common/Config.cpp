#include "Config.h"

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTimer>

#include "ConfigItem.h"
#include "LaserApplication.h"
#include "exception/LaserException.h"
#include "laser/LaserRegister.h"
#include "util/WidgetUtils.h"
#include "util/TypeUtils.h"
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
ConfigItemGroup* Config::FillingLayer::group(nullptr);
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
    loadFillingLayerItems();
    loadPathOptimizationItems();
    loadExportItems();
    loadDeviceItems();
    loadUserReigsters();
    loadSystemRegisters();
    loadDebug();
}

void Config::importFrom(const QString& filename)
{
    // 如果有Config.json文件，则加载该文件，读取后删除。
    QFile configFile(filename);
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
        // 每一个选项组读取各自的配置文件
        configFile.close();
        qLogD << "No valid config.json file found! We will create one.";
        save(true, true);
    }
}

void Config::exportTo(const QString& filename)
{
    try {
        QFile configFile(filename);
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
    }
    catch (...)
    {
        throw new LaserFileException(tr("Save config file error."));
    }
}

void Config::load()
{
    // 如果有Config.json文件，则加载该文件，读取后删除。
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
                group->save(true, true);
            }
        }
        configFile.remove();
    }
    else
    {
        // 每一个选项组读取各自的配置文件
        configFile.close();
        qLogD << "No valid config.json file found! We will create one.";
        for (ConfigItemGroup* group : groups)
        {
            group->load();
        }
    }

}

void Config::save(bool force, bool ignorePreSaveHook)
{
    for (ConfigItemGroup* group : groups)
    {
        group->save(force, ignorePreSaveHook);
    }
}

void Config::restore()
{
}

QString Config::configFilePath(const QString& filename)
{
    QDir dataPath(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation));
    if (!dataPath.exists("CNELaser"))
    {
        dataPath.mkdir("CNELaser");
    }
    dataPath.cd("CNELaser");
    return dataPath.absoluteFilePath(filename);
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
            comboBox->addItem("简体中文", static_cast<int>(QLocale::Chinese));

            QTimer::singleShot(0, 
                [=]() {
                    int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
                    comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
                }
            );
        }
    );
    language->setRetranslateHook(
        [](QWidget* widget, ConfigItem* item)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (!comboBox)
                return;
            comboBox->setItemText(0, tr("English"));
            comboBox->setItemText(1, tr("Chinese"));
        }
    );
    language->setNeedRelaunch(true);

    ConfigItem* unit = group->addConfigItem(
        "unit"
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
    unit->setVisible(false);
}

void Config::loadLayersItems()
{
    ConfigItemGroup* group = new Config::Layers;
    Config::Layers::group = group;

    ConfigItem* maxLayersCount = group->addConfigItem(
        "maxLayersCount"
        , 16
    );
    maxLayersCount->setInputWidgetType(IWT_EditSlider);
    maxLayersCount->setInputWidgetProperty("minimum", 8);
    maxLayersCount->setInputWidgetProperty("maximum", 16);
}

void Config::loadUiItems()
{
    ConfigItemGroup* group = new Config::Ui;
    Config::Ui::group = group;

    ConfigItem* operationButtonIconSize = group->addConfigItem(
        "operationButtonIconSize"
        , 32
    );
    operationButtonIconSize->setInputWidgetProperty("minimum", 16);
    operationButtonIconSize->setInputWidgetProperty("maximum", 64);

    ConfigItem* operationButtonWidth = group->addConfigItem(
        "operationButtonWidth"
        , 60
    );
    operationButtonWidth->setInputWidgetProperty("minimum", 32);
    operationButtonWidth->setInputWidgetProperty("maximum", 120);

    ConfigItem* operationButtonHeight = group->addConfigItem(
        "operationButtonHeight",
        60
    );
    operationButtonHeight->setInputWidgetProperty("minimum", 32);
    operationButtonHeight->setInputWidgetProperty("maximum", 120);

    ConfigItem* operationButtonShowText = group->addConfigItem(
        "operationButtonShowText",
        false,
        DT_BOOL
    );
    operationButtonShowText->setInputWidgetType(IWT_CheckBox);

    ConfigItem* toolButtonSize = group->addConfigItem(
        "toolButtonSize",
        32
    );
    toolButtonSize->setInputWidgetProperty("minimum", 16);
    toolButtonSize->setInputWidgetProperty("maximum", 64);

    ConfigItem* colorButtonWidth = group->addConfigItem(
        "colorButtonWidth",
        30
    );
    colorButtonWidth->setInputWidgetProperty("minimum", 20);
    colorButtonWidth->setInputWidgetProperty("maximum", 60);

    ConfigItem* colorButtonHeight = group->addConfigItem(
        "colorButtonHeight",
        30
    );
    colorButtonHeight->setInputWidgetProperty("minimum", 20);
    colorButtonHeight->setInputWidgetProperty("maximum", 60);

    ConfigItem* gridContrast = group->addConfigItem(
        "gridContrast",
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

            comboBox->addItem(ltr("Off"), 0);
            comboBox->addItem(ltr("Low Contrast"), 1);
            comboBox->addItem(ltr("Medium Contrast"), 2);
            comboBox->addItem(ltr("High Contrast"), 3);

            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );

	ConfigItem* gridShapeDistance = group->addConfigItem(
		"gridShapeDistance",
		3
	);
    gridShapeDistance->setInputWidgetProperty("textTemplate", "%1px");
	gridShapeDistance->setInputWidgetProperty("minimum", 0);
	gridShapeDistance->setInputWidgetProperty("maximum", 10);

	ConfigItem* objectShapeDistance = group->addConfigItem(
		"objectShapeDistance",
		5
	);
    objectShapeDistance->setInputWidgetProperty("textTemplate", "%1px");
	objectShapeDistance->setInputWidgetProperty("minimum", 0);
	objectShapeDistance->setInputWidgetProperty("maximum", 10);

	ConfigItem* clickSelectionTolerance = group->addConfigItem(
		"clickSelectionTolerance",
		5
	);
    clickSelectionTolerance->setInputWidgetProperty("textTemplate", "%1px");
	clickSelectionTolerance->setInputWidgetProperty("minimum", 0);
	clickSelectionTolerance->setInputWidgetProperty("maximum", 10);

	ConfigItem* visualGridSpacing = group->addConfigItem(
		"visualGridSpacing",
		10,
        DT_INT
	);
    visualGridSpacing->setInputWidgetProperty("textTemplate", "%1mm");
	visualGridSpacing->setInputWidgetProperty("minimum", 1);
	visualGridSpacing->setInputWidgetProperty("maximum", 50);

   
    ConfigItem* validMaxRegion = group->addConfigItem(
        "validMaxRegion",
        3000,
        DT_INT
    );
    validMaxRegion->setInputWidgetProperty("textTemplate", "%1mm");
    validMaxRegion->setInputWidgetProperty("minimum", 0);
    validMaxRegion->setInputWidgetProperty("maximum", 10000);

    ConfigItem* splitterHandleWidth = group->addConfigItem(
        "splitterHandleWidth",
        1
    );
    splitterHandleWidth->setInputWidgetProperty("textTemplate", "%1mm");
    splitterHandleWidth->setInputWidgetProperty("minimum", 1);
    splitterHandleWidth->setInputWidgetProperty("maximum", 20);

    ConfigItem* autoRepeatDelay = group->addConfigItem(
        "autoRepeatDelay",
        1000
    );
    autoRepeatDelay->setInputWidgetProperty("textTemplate", "%1ms");
    autoRepeatDelay->setInputWidgetProperty("minimum", 0);
    autoRepeatDelay->setInputWidgetProperty("maximum", 2000);

    ConfigItem* showDocumentBoundingRect = group->addConfigItem(
        "showDocumentBoundingRect",
        false,
        DT_BOOL
    );
    showDocumentBoundingRect->setInputWidgetType(IWT_CheckBox);

    ConfigItem* laserCursorTimeout = group->addConfigItem(
        "laserCursorTimeout",
        3000,
        DT_INT
    );
    laserCursorTimeout->setInputWidgetProperty("textTemplate", "%1ms");
    laserCursorTimeout->setInputWidgetProperty("minimum", 0);
    laserCursorTimeout->setInputWidgetProperty("maximum", 10000);
}

void Config::loadCuttingLayerItems()
{
    ConfigItemGroup* group = new Config::CuttingLayer;
    Config::CuttingLayer::group = group;

    ConfigItem* runSpeed = group->addConfigItem(
        "runSpeed",
        60000,
        DT_INT
    );
    runSpeed->setInputWidgetType(IWT_FloatEditSlider);
    runSpeed->setInputWidgetProperty("decimals", 3);
    runSpeed->setInputWidgetProperty("minimum", 1);
    runSpeed->setInputWidgetProperty("maximum", 1000);
    runSpeed->setInputWidgetProperty("step", 0.001);
    runSpeed->setInputWidgetProperty("maximumLineEditWidth", 60);

    ConfigItem* minPower = group->addConfigItem(
        "minPower",
        700,
        DT_INT
    );
    minPower->setInputWidgetType(IWT_FloatEditSlider);
    minPower->setInputWidgetProperty("decimals", 1);
    minPower->setInputWidgetProperty("minimum", 0);
    minPower->setInputWidgetProperty("maximum", 100);
    minPower->setInputWidgetProperty("step", 0.1);
    minPower->setInputWidgetProperty("textTemplate", "%1%");

    ConfigItem* maxPower = group->addConfigItem(
        "maxPower",
        1000,
        DT_INT
    );
    maxPower->setInputWidgetType(IWT_FloatEditSlider);
    maxPower->setInputWidgetProperty("decimals", 1);
    maxPower->setInputWidgetProperty("minimum", 0);
    maxPower->setInputWidgetProperty("maximum", 100);
    maxPower->setInputWidgetProperty("step", 0.1);
    maxPower->setInputWidgetProperty("textTemplate", "%1%");
}

void Config::loadEngravingLayerItems()
{
    ConfigItemGroup* group = new Config::EngravingLayer;
    Config::EngravingLayer::group = group;

    ConfigItem* runSpeed = group->addConfigItem(
        "runSpeed",
        60000,
        DT_INT
    );
    runSpeed->setInputWidgetType(IWT_FloatEditSlider);
    runSpeed->setInputWidgetProperty("decimals", 3);
    runSpeed->setInputWidgetProperty("minimum", 1);
    runSpeed->setInputWidgetProperty("maximum", 1000);
    runSpeed->setInputWidgetProperty("step", 0.001);
    runSpeed->setInputWidgetProperty("maximumLineEditWidth", 60);

    ConfigItem* laserPower = group->addConfigItem(
        "laserPower",
        700,
        DT_INT
    );
    laserPower->setInputWidgetType(IWT_FloatEditSlider);
    laserPower->setInputWidgetProperty("decimals", 1);
    laserPower->setInputWidgetProperty("minimum", 0);
    laserPower->setInputWidgetProperty("maximum", 100);
    laserPower->setInputWidgetProperty("step", 0.1);
    laserPower->setInputWidgetProperty("textTemplate", "%1%");

    ConfigItem* minPower= group->addConfigItem(
        "minPower",
        700,
        DT_INT
    );
    minPower->setInputWidgetType(IWT_FloatEditSlider);
    minPower->setInputWidgetProperty("step", 0.1);
    minPower->setInputWidgetProperty("decimals", 1);
    minPower->setInputWidgetProperty("minimum", 0);
    minPower->setInputWidgetProperty("maximum", 100);
    minPower->setInputWidgetProperty("textTemplate", "%1%");

    ConfigItem* maxPower= group->addConfigItem(
        "maxPower",
        1000,
        DT_INT
    );
    maxPower->setInputWidgetType(IWT_FloatEditSlider);
    maxPower->setInputWidgetProperty("step", 0.1);
    maxPower->setInputWidgetProperty("decimals", 1);
    maxPower->setInputWidgetProperty("minimum", 0);
    maxPower->setInputWidgetProperty("maximum", 100);
    maxPower->setInputWidgetProperty("textTemplate", "%1%");

    ConfigItem* rowInterval = group->addConfigItem(
        "rowInterval",
        70,
        DT_INT
    );
    rowInterval->setInputWidgetProperty("minimum", 0);
    rowInterval->setInputWidgetProperty("maximum", 1000);
    rowInterval->setInputWidgetProperty("textTemplate", "%1μm");

    group->addConfigItem(
        "useHalftone",
        true,
        DT_BOOL
    );

    ConfigItem* halftoneAngles = group->addConfigItem(
        "halftoneAngles",
        45,
        DT_REAL
    );
    halftoneAngles->setInputWidgetType(IWT_DoubleSpinBox);
    halftoneAngles->setInputWidgetProperty("minimum", 0);
    halftoneAngles->setInputWidgetProperty("maximum", 90);

    ConfigItem* halftoneGridSize = group->addConfigItem(
        "halftoneGridSize",
        12,
        DT_INT
    );
    halftoneGridSize->setInputWidgetType(IWT_EditSlider);
    halftoneGridSize->setInputWidgetProperty("minimum", 4);
    halftoneGridSize->setInputWidgetProperty("maximum", 100);

    ConfigItem* lpi = group->addConfigItem(
        "LPI",
        100,
        DT_INT
    );
    lpi->setInputWidgetProperty("minimum", 1);
    lpi->setInputWidgetProperty("maximum", 1200);

    ConfigItem* dpi = group->addConfigItem(
        "DPI",
        600,
        DT_INT
    );
    dpi->setInputWidgetProperty("minimum", 1);
    dpi->setInputWidgetProperty("maximum", 1200);

    ConfigItem* enableCutting = group->addConfigItem(
        "enableCutting",
        true,
        DT_BOOL
    );
}

void Config::loadFillingLayerItems()
{
    ConfigItemGroup* group = new Config::FillingLayer;
    Config::FillingLayer::group = group;

    ConfigItem* runSpeed = group->addConfigItem(
        "runSpeed",
        30000,
        DT_INT
    );
    runSpeed->setInputWidgetType(IWT_FloatEditSlider);
    runSpeed->setInputWidgetProperty("step", 0.001);
    runSpeed->setInputWidgetProperty("decimals", 3);
    runSpeed->setInputWidgetProperty("minimum", 1);
    runSpeed->setInputWidgetProperty("maximum", 1000);
    runSpeed->setInputWidgetProperty("maximumLineEditWidth", 60);
    runSpeed->setVisible(false);

    ConfigItem* minPower= group->addConfigItem(
        "minPower",
        40,
        DT_INT
    );
    minPower->setInputWidgetType(IWT_FloatEditSlider);
    minPower->setInputWidgetProperty("step", 0.1);
    minPower->setInputWidgetProperty("decimals", 1);
    minPower->setInputWidgetProperty("minimum", 0);
    minPower->setInputWidgetProperty("maximum", 100);
    minPower->setInputWidgetProperty("textTemplate", "%1%");
    minPower->setVisible(false);

    ConfigItem* maxPower= group->addConfigItem(
        "maxPower",
        120,
        DT_INT
    );
    maxPower->setInputWidgetType(IWT_FloatEditSlider);
    maxPower->setInputWidgetProperty("step", 0.1);
    maxPower->setInputWidgetProperty("decimals", 1);
    maxPower->setInputWidgetProperty("minimum", 0);
    maxPower->setInputWidgetProperty("maximum", 100);
    maxPower->setInputWidgetProperty("textTemplate", "%1%");
    maxPower->setVisible(false);

    ConfigItem* rowInterval = group->addConfigItem(
        "rowInterval",
        70,
        DT_INT
    );
    rowInterval->setInputWidgetProperty("minimum", 0);
    rowInterval->setInputWidgetProperty("maximum", 1000);
    rowInterval->setInputWidgetProperty("textTemplate", "%1μm");
    rowInterval->setVisible(false);

    ConfigItem* enableCutting = group->addConfigItem(
        "enableCutting",
        true,
        DT_BOOL
    );

    ConfigItem* fillingType = group->addConfigItem(
        "fillingType",
        1,
        DT_INT
    );
    fillingType->setInputWidgetType(IWT_ComboBox);
    fillingType->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item, InputWidgetWrapper* wrapper)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (!comboBox)
                return;

            comboBox->addItem(tr("Line"), 0);
            comboBox->addItem(tr("Pixel"), 1);

            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );
    fillingType->setVisible(false);
}

void Config::loadPathOptimizationItems()
{
    ConfigItemGroup* group = new Config::PathOptimization;
    Config::PathOptimization::group = group;

    ConfigItem* maxStartingPoints = group->addConfigItem(
        "maxStartingPoints",
        8
    );
    maxStartingPoints->setInputWidgetProperty("minimum", 1);
    maxStartingPoints->setInputWidgetProperty("maximum", 16);

    ConfigItem* maxGroupSize = group->addConfigItem(
        "maxGroupSize",
        10
    );
    maxGroupSize->setInputWidgetProperty("minimum", 1);
    maxGroupSize->setInputWidgetProperty("maximum", 100);

    ConfigItem* groupingOrientation = group->addConfigItem(
        "groupingOrientation",
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

            comboBox->addItem(ltr("Horizontal"), 1);
            comboBox->addItem(ltr("Vertical"), 2);

            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );

    ConfigItem* groupingGridInterval = group->addConfigItem(
        "groupingGridInterval",
        30000,
        DT_INT
    );
    groupingGridInterval->setInputWidgetType(IWT_FloatEditSlider);
    groupingGridInterval->setInputWidgetProperty("step", 0.001);
    groupingGridInterval->setInputWidgetProperty("decimals", 0);
    groupingGridInterval->setInputWidgetProperty("maximumLineEditWidth", 75);
    groupingGridInterval->setInputWidgetProperty("step", 10);
    groupingGridInterval->setInputWidgetProperty("page", 10);
    groupingGridInterval->setInputWidgetProperty("minimum", 1);
    groupingGridInterval->setInputWidgetProperty("maximum", 1000);

    ConfigItem* searchingXYWeight = group->addConfigItem(
        "searchingXYWeight",
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
        5.0,
        DT_REAL
    );
    maxAnglesDiff->setInputWidgetProperty("minimum", 1.0);
    maxAnglesDiff->setInputWidgetProperty("maximum", 90.0);

    ConfigItem* maxIntervalDistance = group->addConfigItem(
        "maxIntervalDistance",
        40
    );
    maxIntervalDistance->setInputWidgetProperty("minimum", 1);
    maxIntervalDistance->setInputWidgetProperty("maximum", 1000);

    ConfigItem* enableSmallDiagonal = group->addConfigItem(
        "enableSmallDiagonal",
        false,
        DT_BOOL
    );

    QVariant smallDiagonalLimitationVar;
    SmallDiagonalLimitation* limitation = new SmallDiagonalLimitation;
    smallDiagonalLimitationVar.setValue(limitation);
    qLogD << "small diagonal limitation type: " << smallDiagonalLimitationVar.userType();
    ConfigItem* smallDiagonalLimitation = group->addConfigItem(
        "smallDiagonalLimitation",
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
                limitation->fromJson(json["value"].toObject());
            }
        }
    );
    connect(enableSmallDiagonal, &ConfigItem::valueChanged,
        [=](const QVariant& value, void* senderPtr) {
            bool enabled = value.toBool();
            smallDiagonalLimitation->setEnabled(enabled);
        }
    );

    ConfigItem* curveFlatteningThreshold = group->addConfigItem(
        "curveFlatteningThreshold",
        20,
        DT_REAL
    );
    curveFlatteningThreshold->setInputWidgetProperty("minimum", 0);
    curveFlatteningThreshold->setInputWidgetProperty("maximum", 1000);

    ConfigItem* gaussianFactorA = group->addConfigItem(
        "gaussianFactorA",
        1.25,
        DT_REAL
    );
    gaussianFactorA->setInputWidgetProperty("minimum", 0);
    gaussianFactorA->setInputWidgetProperty("maximum", 1000);
    gaussianFactorA->setInputWidgetProperty("decimals", 3);
}

void Config::loadDeviceItems()
{
    ConfigItemGroup* group = new Config::Device;
    Config::Device::group = group;

    group->addConfigItem(
        "autoConnectFirstCOM",
        true,
        DT_BOOL
    );

    ConfigItem* startFrom = group->addConfigItem(
        "startFrom",
        0,
        DT_INT
    );
    startFrom->setInputWidgetType(IWT_ComboBox);
    startFrom->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item, InputWidgetWrapper* wrapper)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (!comboBox)
                return;

            comboBox->addItem(ltr("Current Position"), SFT_CurrentPosition);
            comboBox->addItem(ltr("User Origin"), SFT_UserOrigin);
            comboBox->addItem(ltr("Absolute Coords"), SFT_AbsoluteCoords);

            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );

    ConfigItem* jobOrigin = group->addConfigItem(
        "jobOrigin",
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
            return value;
        }
    );
    connect(startFrom, &ConfigItem::valueChanged,
        [=](const QVariant& value, void* senderPtr) {
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
        true,
        DT_BOOL
    );

    ConfigItem* yEnabled = group->addConfigItem(
        "yEnabled",
        true,
        DT_BOOL
    );

    ConfigItem* zEnabled = group->addConfigItem(
        "zEnabled",
        true,
        DT_BOOL
    );

    ConfigItem* userOrigin1 = group->addConfigItem(
        "userOrigin1",
        QPoint(0, 0),
        DT_POINT
    );
    userOrigin1->setInputWidgetProperty("minimum", -1000000);
    userOrigin1->setInputWidgetProperty("maximum", 1000000);
    userOrigin1->setInputWidgetProperty("decimals", 3);
    userOrigin1->setValueFromWidgetHook(
        [](QWidget* widget, const QVariant& value)
        {
            QPoint pt = value.toPoint();
            return QVariant(pt * 1000);
        }
    );
    userOrigin1->setUpdateWidgetValueHook(
        [](QWidget* widget, const QVariant& value)
        {
            Vector2DWidget* v2w = qobject_cast<Vector2DWidget*>(widget);
            QPoint pt = value.toPoint();
            v2w->setValue(pt * 0.001);
            return v2w->toPointF();
        }
    );
    userOrigin1->setToJsonHook(
        [=](const ConfigItem* item) {
            QPoint pt = item->value().toPoint();
            QPoint defPt = item->defaultValue().toPoint();
            QJsonObject jsonObj;
            jsonObj["value"] = typeUtils::point2Json(pt);
            jsonObj["defaultValue"] = typeUtils::point2Json(defPt);
            return jsonObj;
        }
    );
    userOrigin1->setFromJsonHook(
        [=](QVariant& value, QVariant& defaultValue, const QJsonObject& json, ConfigItem* item) {
            if (json.contains("value"))
            {
                value = typeUtils::json2Point(json["value"]);
            }
            if (json.contains("defaultValue"))
            {
                defaultValue = typeUtils::json2Point(json["defaultValue"]);
            }
        }
    );

    ConfigItem* userOrigin2 = group->addConfigItem(
        "userOrigin2",
        QPoint(0, 0),
        DT_POINT
    );
    userOrigin2->setInputWidgetProperty("minimum", -1000000);
    userOrigin2->setInputWidgetProperty("maximum", 1000000);
    userOrigin2->setInputWidgetProperty("decimals", 3);
    userOrigin2->setValueFromWidgetHook(
        [](QWidget* widget, const QVariant& value)
        {
            QPoint pt = value.toPoint();
            return QVariant(pt * 1000);
        }
    );
    userOrigin2->setUpdateWidgetValueHook(
        [](QWidget* widget, const QVariant& value)
        {
            Vector2DWidget* v2w = qobject_cast<Vector2DWidget*>(widget);
            QPoint pt = value.toPoint();
            v2w->setValue(pt * 0.001);
            return v2w->toPointF();
        }
    );
    userOrigin2->setToJsonHook(
        [=](const ConfigItem* item) {
            QPoint pt = item->value().toPoint();
            QPoint defPt = item->defaultValue().toPoint();
            QJsonObject jsonObj;
            jsonObj["value"] = typeUtils::point2Json(pt);
            jsonObj["defaultValue"] = typeUtils::point2Json(defPt);
            return jsonObj;
        }
    );
    userOrigin2->setFromJsonHook(
        [=](QVariant& value, QVariant& defaultValue, const QJsonObject& json, ConfigItem* item) {
            if (json.contains("value"))
            {
                value = typeUtils::json2Point(json["value"]);
            }
            if (json.contains("defaultValue"))
            {
                defaultValue = typeUtils::json2Point(json["defaultValue"]);
            }
        }
    );

    ConfigItem* userOrigin3 = group->addConfigItem(
        "userOrigin3",
        QPoint(0, 0),
        DT_POINT
    );
    userOrigin3->setInputWidgetProperty("minimum", -1000000);
    userOrigin3->setInputWidgetProperty("maximum", 1000000);
    userOrigin3->setInputWidgetProperty("decimals", 3);
    userOrigin3->setValueFromWidgetHook(
        [](QWidget* widget, const QVariant& value)
        {
            QPoint pt = value.toPoint();
            return QVariant(pt * 1000);
        }
    );
    userOrigin3->setUpdateWidgetValueHook(
        [](QWidget* widget, const QVariant& value)
        {
            Vector2DWidget* v2w = qobject_cast<Vector2DWidget*>(widget);
            QPoint pt = value.toPoint();
            v2w->setValue(pt * 0.001);
            return v2w->toPointF();
        }
    );
    userOrigin3->setToJsonHook(
        [=](const ConfigItem* item) {
            QPoint pt = item->value().toPoint();
            QPoint defPt = item->defaultValue().toPoint();
            QJsonObject jsonObj;
            jsonObj["value"] = typeUtils::point2Json(pt);
            jsonObj["defaultValue"] = typeUtils::point2Json(defPt);
            return jsonObj;
        }
    );
    userOrigin3->setFromJsonHook(
        [=](QVariant& value, QVariant& defaultValue, const QJsonObject& json, ConfigItem* item) {
            if (json.contains("value"))
            {
                value = typeUtils::json2Point(json["value"]);
            }
            if (json.contains("defaultValue"))
            {
                defaultValue = typeUtils::json2Point(json["defaultValue"]);
            }
        }
    );

    ConfigItem* userOriginSelected = group->addConfigItem(
        "userOriginSelected",
        0,
        DT_INT
    );
    userOriginSelected->setInputWidgetType(IWT_ComboBox);
    userOriginSelected->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item, InputWidgetWrapper* wrapper)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (!comboBox)
                return;

            comboBox->addItem(ltr("User Origin 1"), 0);
            comboBox->addItem(ltr("User Origin 2"), 1);
            comboBox->addItem(ltr("User Origin 3"), 2);

            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );

    ConfigItem* redLightOffset = group->addConfigItem(
        "redLightOffset",
        QPointF(0, 0),
        DT_POINT
    );
    redLightOffset->setVisible(false);

    ConfigItem* zReverseDirection = group->addConfigItem(
        "zReverseDirection",
        false,
        DT_BOOL
    );

    ConfigItem* zFocalLength = group->addConfigItem(
        "zFocalLength",
        200000,
        DT_INT
    );
    zFocalLength->setInputWidgetType(IWT_FloatEditSlider);
    zFocalLength->setInputWidgetProperty("step", 0.001);
    zFocalLength->setInputWidgetProperty("decimals", 3);
    zFocalLength->setInputWidgetProperty("maximumLineEditWidth", 75);
    zFocalLength->setInputWidgetProperty("page", 10);
    zFocalLength->setInputWidgetProperty("minimum", 1);
    zFocalLength->setInputWidgetProperty("maximum", 2000);
    zFocalLength->setNeedRelaunch(true);

    ConfigItem* calibrationBlockThickness = group->addConfigItem(
        "calibrationBlockThickness",
        3000,
        DT_INT
    );
    calibrationBlockThickness->setInputWidgetType(IWT_FloatEditSlider);
    calibrationBlockThickness->setInputWidgetProperty("step", 0.001);
    calibrationBlockThickness->setInputWidgetProperty("decimals", 3);
    calibrationBlockThickness->setInputWidgetProperty("maximumLineEditWidth", 75);
    calibrationBlockThickness->setInputWidgetProperty("textTemplate", "%1");
    calibrationBlockThickness->setInputWidgetProperty("page", 1);
    calibrationBlockThickness->setInputWidgetProperty("minimum", 0);
    calibrationBlockThickness->setInputWidgetProperty("maximum", 10);
}

void Config::loadUserReigsters()
{
    ConfigItemGroup* group = new Config::UserRegister;
    group->setLazy(true);
    Config::UserRegister::group = group;

    ConfigItem* head = group->addConfigItem(
        "head",
        0x12345678,
        DT_INT
    );
    head->setInputWidgetType(IWT_LineEdit);
    head->setInputWidgetProperty("readOnly", true);
    head->setVisible(false);

    ConfigItem* accMode = group->addConfigItem(
        "accMode",
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
    accMode->setVisible(false);

    ConfigItem* cuttingMoveSpeed = group->addConfigItem(
        "cuttingMoveSpeed",
        200000,
        DT_INT
    );
    cuttingMoveSpeed->setInputWidgetType(IWT_FloatEditSlider);
    cuttingMoveSpeed->setInputWidgetProperty("step", 0.001);
    cuttingMoveSpeed->setInputWidgetProperty("decimals", 3);
    cuttingMoveSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    cuttingMoveSpeed->setInputWidgetProperty("page", 10);
    cuttingMoveSpeed->setInputWidgetProperty("minimum", 1);
    cuttingMoveSpeed->setInputWidgetProperty("maximum", 1000);

    ConfigItem* cuttingMoveAcc = group->addConfigItem(
        "cuttingMoveAcc",
        2500000,
        DT_INT
    );
    cuttingMoveAcc->setInputWidgetType(IWT_FloatEditSlider);
    cuttingMoveAcc->setInputWidgetProperty("step", 0.001);
    cuttingMoveAcc->setInputWidgetProperty("decimals", 3);
    cuttingMoveAcc->setInputWidgetProperty("maximumLineEditWidth", 75);
    cuttingMoveAcc->setInputWidgetProperty("page", 10);
    cuttingMoveAcc->setInputWidgetProperty("minimum", 1);
    cuttingMoveAcc->setInputWidgetProperty("maximum", 20000);

    ConfigItem* cuttingTurnSpeed = group->addConfigItem(
        "cuttingTurnSpeed",
        8000,
        DT_INT
    );
    cuttingTurnSpeed->setInputWidgetType(IWT_FloatEditSlider);
    cuttingTurnSpeed->setInputWidgetProperty("step", 0.001);
    cuttingTurnSpeed->setInputWidgetProperty("decimals", 3);
    cuttingTurnSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    cuttingTurnSpeed->setInputWidgetProperty("page", 10);
    cuttingTurnSpeed->setInputWidgetProperty("minimum", 1);
    cuttingTurnSpeed->setInputWidgetProperty("maximum", 100);

    ConfigItem* cuttingTurnAcc = group->addConfigItem(
        "cuttingTurnAcc",
        400000,
        DT_INT
    );
    cuttingTurnAcc->setInputWidgetType(IWT_FloatEditSlider);
    cuttingTurnAcc->setInputWidgetProperty("step", 0.001);
    cuttingTurnAcc->setInputWidgetProperty("decimals", 3);
    cuttingTurnAcc->setInputWidgetProperty("maximumLineEditWidth", 75);
    cuttingTurnAcc->setInputWidgetProperty("page", 10);
    cuttingTurnAcc->setInputWidgetProperty("minimum", 1);
    cuttingTurnAcc->setInputWidgetProperty("maximum", 5000);

    ConfigItem* cuttingWorkAcc = group->addConfigItem(
        "cuttingWorkAcc",
        800000,
        DT_INT
    );
    cuttingWorkAcc->setInputWidgetType(IWT_FloatEditSlider);
    cuttingWorkAcc->setInputWidgetProperty("step", 0.001);
    cuttingWorkAcc->setInputWidgetProperty("decimals", 3);
    cuttingWorkAcc->setInputWidgetProperty("maximumLineEditWidth", 75);
    cuttingWorkAcc->setInputWidgetProperty("page", 10);
    cuttingWorkAcc->setInputWidgetProperty("minimum", 1);
    cuttingWorkAcc->setInputWidgetProperty("maximum", 10000);

    ConfigItem* cuttingMoveSpeedFactor = group->addConfigItem(
        "cuttingMoveSpeedFactor",
        100
    );
    cuttingMoveSpeedFactor->setInputWidgetProperty("maximumLineEditWidth", 75);
    cuttingMoveSpeedFactor->setInputWidgetProperty("minimum", 1);
    cuttingMoveSpeedFactor->setInputWidgetProperty("maximum", 200);

    ConfigItem* cuttingWorkSpeedFactor = group->addConfigItem(
        "cuttingWorkSpeedFactor",
        100
    );
    cuttingWorkSpeedFactor->setInputWidgetProperty("maximumLineEditWidth", 75);
    cuttingWorkSpeedFactor->setInputWidgetProperty("minimum", 1);
    cuttingWorkSpeedFactor->setInputWidgetProperty("maximum", 200);

    ConfigItem* cuttingSpotSize = group->addConfigItem(
        "cuttingSpotSize",
        1000,
        DT_INT
    );
    cuttingSpotSize->setInputWidgetProperty("maximumLineEditWidth", 75);
    cuttingSpotSize->setInputWidgetProperty("minimum", 1);
    cuttingSpotSize->setInputWidgetProperty("maximum", 1000);
    cuttingSpotSize->setVisible(false);

    ConfigItem* scanXStartSpeed = group->addConfigItem(
        "scanXStartSpeed",
        30000,
        DT_INT
    );
    scanXStartSpeed->setInputWidgetType(IWT_FloatEditSlider);
    scanXStartSpeed->setInputWidgetProperty("step", 0.001);
    scanXStartSpeed->setInputWidgetProperty("decimals", 3);
    scanXStartSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    scanXStartSpeed->setInputWidgetProperty("page", 10);
    scanXStartSpeed->setInputWidgetProperty("minimum", 1);
    scanXStartSpeed->setInputWidgetProperty("maximum", 100);

    ConfigItem* scanYStartSpeed = group->addConfigItem(
        "scanYStartSpeed",
        15000,
        DT_INT
    );
    scanYStartSpeed->setInputWidgetType(IWT_FloatEditSlider);
    scanYStartSpeed->setInputWidgetProperty("step", 0.001);
    scanYStartSpeed->setInputWidgetProperty("decimals", 3);
    scanYStartSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    scanYStartSpeed->setInputWidgetProperty("page", 10);
    scanYStartSpeed->setInputWidgetProperty("minimum", 1);
    scanYStartSpeed->setInputWidgetProperty("maximum", 100);

    ConfigItem* scanXAcc = group->addConfigItem(
        "scanXAcc",
        6000000,
        DT_INT
    );
    scanXAcc->setInputWidgetType(IWT_FloatEditSlider);
    scanXAcc->setInputWidgetProperty("step", 0.001);
    scanXAcc->setInputWidgetProperty("decimals", 3);
    scanXAcc->setInputWidgetProperty("maximumLineEditWidth", 75);
    scanXAcc->setInputWidgetProperty("page", 10);
    scanXAcc->setInputWidgetProperty("minimum", 1);
    scanXAcc->setInputWidgetProperty("maximum", 20000);

    ConfigItem* scanYAcc = group->addConfigItem(
        "scanYAcc",
        1000000,
        DT_INT
    );
    scanYAcc->setInputWidgetType(IWT_FloatEditSlider);
    scanYAcc->setInputWidgetProperty("step", 0.001);
    scanYAcc->setInputWidgetProperty("decimals", 3);
    scanYAcc->setInputWidgetProperty("maximumLineEditWidth", 75);
    scanYAcc->setInputWidgetProperty("page", 10);
    scanYAcc->setInputWidgetProperty("minimum", 1);
    scanYAcc->setInputWidgetProperty("maximum", 20000);

    ConfigItem* scanRowSpeed = group->addConfigItem(
        "scanRowSpeed",
        50000,
        DT_INT
    );
    scanRowSpeed->setInputWidgetType(IWT_FloatEditSlider);
    scanRowSpeed->setInputWidgetProperty("step", 0.001);
    scanRowSpeed->setInputWidgetProperty("decimals", 3);
    scanRowSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    scanRowSpeed->setInputWidgetProperty("page", 10);
    scanRowSpeed->setInputWidgetProperty("minimum", 1);
    scanRowSpeed->setInputWidgetProperty("maximum", 500);

    ConfigItem* scanLaserFrequency = group->addConfigItem(
        "scanLaserFrequency",
        20000,
        DT_INT
    );
    scanLaserFrequency->setInputWidgetProperty("maximumLineEditWidth", 75);
    scanLaserFrequency->setInputWidgetProperty("page", 100);
    scanLaserFrequency->setInputWidgetProperty("minimum", 5000);
    scanLaserFrequency->setInputWidgetProperty("maximum", 100000);

    ConfigItem* scanReturnError = group->addConfigItem(
        "scanReturnError",
        0,
        DT_INT
    );
    scanReturnError->setInputWidgetType(IWT_FloatEditSlider);
    scanReturnError->setInputWidgetProperty("step", 0.001);
    scanReturnError->setInputWidgetProperty("decimals", 3);
    scanReturnError->setInputWidgetProperty("maximumLineEditWidth", 75);
    scanReturnError->setInputWidgetProperty("page", 1);
    scanReturnError->setInputWidgetProperty("minimum", -50);
    scanReturnError->setInputWidgetProperty("maximum", 50);

    ConfigItem* scanLaserPower = group->addConfigItem(
        "scanLaserPower",
        1200,
        DT_INT
    );
    scanLaserPower->setInputWidgetType(IWT_FloatEditSlider);
    scanLaserPower->setInputWidgetProperty("step", 0.1);
    scanLaserPower->setInputWidgetProperty("decimals", 1);
    scanLaserPower->setInputWidgetProperty("textTemplate", "%1%");
    scanLaserPower->setInputWidgetProperty("maximumLineEditWidth", 75);
    scanLaserPower->setInputWidgetProperty("page", 10);
    scanLaserPower->setInputWidgetProperty("minimum", 1);
    scanLaserPower->setInputWidgetProperty("maximum", 100);

    ConfigItem* scanXResetEnabled = group->addConfigItem(
        "scanXResetEnabled",
        true,
        DT_INT
    );
    scanXResetEnabled->setInputWidgetType(IWT_CheckBox);

    ConfigItem* scanYResetEnabled = group->addConfigItem(
        "scanYResetEnabled",
        true,
        DT_INT
    );
    scanYResetEnabled->setInputWidgetType(IWT_CheckBox);

    ConfigItem* scanZResetEnabled = group->addConfigItem(
        "scanZResetEnabled",
        true,
        DT_INT
    );
    scanZResetEnabled->setInputWidgetType(IWT_CheckBox);

    ConfigItem* resetSpeed = group->addConfigItem(
        "resetSpeed",
        50000,
        DT_INT
    );
    resetSpeed->setInputWidgetType(IWT_FloatEditSlider);
    resetSpeed->setInputWidgetProperty("step", 0.001);
    resetSpeed->setInputWidgetProperty("decimals", 3);
    resetSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    resetSpeed->setInputWidgetProperty("textTemplate", "%1");
    resetSpeed->setInputWidgetProperty("page", 10);
    resetSpeed->setInputWidgetProperty("minimum", 1);
    resetSpeed->setInputWidgetProperty("maximum", 100);

    ConfigItem* scanReturnPos = group->addConfigItem(
        "scanReturnPos",
        0
    );
    scanReturnPos->setInputWidgetProperty("maximumLineEditWidth", 75);
    scanReturnPos->setInputWidgetProperty("page", 1000);
    scanReturnPos->setInputWidgetProperty("minimum", 0);
    scanReturnPos->setInputWidgetProperty("maximum", 100000);

    ConfigItem* backlashXInterval = group->addConfigItem(
        "backlashXInterval",
        0,
        DT_INT
    );
    backlashXInterval->setInputWidgetType(IWT_FloatEditSlider);
    backlashXInterval->setInputWidgetProperty("step", 0.001);
    backlashXInterval->setInputWidgetProperty("decimals", 3);
    backlashXInterval->setInputWidgetProperty("maximumLineEditWidth", 75);
    backlashXInterval->setInputWidgetProperty("page", 1);
    backlashXInterval->setInputWidgetProperty("minimum", -20);
    backlashXInterval->setInputWidgetProperty("maximum", 20);

    ConfigItem* backlashYInterval = group->addConfigItem(
        "backlashYInterval",
        0,
        DT_INT
    );
    backlashYInterval->setInputWidgetType(IWT_FloatEditSlider);
    backlashYInterval->setInputWidgetProperty("step", 0.001);
    backlashYInterval->setInputWidgetProperty("decimals", 3);
    backlashYInterval->setInputWidgetProperty("maximumLineEditWidth", 75);
    backlashYInterval->setInputWidgetProperty("page", 1);
    backlashYInterval->setInputWidgetProperty("minimum", -20);
    backlashYInterval->setInputWidgetProperty("maximum", 20);

    ConfigItem* backlashZInterval = group->addConfigItem(
        "backlashZInterval",
        0,
        DT_INT
    );
    backlashZInterval->setInputWidgetType(IWT_FloatEditSlider);
    backlashZInterval->setInputWidgetProperty("maximumLineEditWidth", 75);
    backlashZInterval->setInputWidgetProperty("step", 0.001);
    backlashZInterval->setInputWidgetProperty("page", 10);
    backlashZInterval->setInputWidgetProperty("minimum", -20);
    backlashZInterval->setInputWidgetProperty("maximum", 20);
    backlashZInterval->setInputWidgetProperty("decimals", 3);

    ConfigItem* defaultRunSpeed = group->addConfigItem(
        "defaultRunSpeed",
        120,
        DT_INT
    );
    defaultRunSpeed->setInputWidgetType(IWT_FloatEditSlider);
    defaultRunSpeed->setInputWidgetProperty("step", 0.001);
    defaultRunSpeed->setInputWidgetProperty("decimals", 3);
    defaultRunSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    defaultRunSpeed->setInputWidgetProperty("textTemplate", "%1");
    defaultRunSpeed->setInputWidgetProperty("page", 10);
    defaultRunSpeed->setInputWidgetProperty("minimum", 1);
    defaultRunSpeed->setInputWidgetProperty("maximum", 1000);

    ConfigItem* defaultMaxCuttingPower = group->addConfigItem(
        "defaultMaxCuttingPower",
        100,
        DT_INT
    );
    defaultMaxCuttingPower->setInputWidgetType(IWT_FloatEditSlider);
    defaultMaxCuttingPower->setInputWidgetProperty("step", 0.1);
    defaultMaxCuttingPower->setInputWidgetProperty("decimals", 1);
    defaultMaxCuttingPower->setInputWidgetProperty("textTemplate", "%1%");
    defaultMaxCuttingPower->setInputWidgetProperty("maximumLineEditWidth", 75);
    defaultMaxCuttingPower->setInputWidgetProperty("page", 10);
    defaultMaxCuttingPower->setInputWidgetProperty("minimum", 0);
    defaultMaxCuttingPower->setInputWidgetProperty("maximum", 100);

    ConfigItem* defaultMinCuttingPower = group->addConfigItem(
        "defaultMinCuttingPower",
        90,
        DT_INT
    );
    defaultMinCuttingPower->setInputWidgetType(IWT_FloatEditSlider);
    defaultMinCuttingPower->setInputWidgetProperty("step", 0.1);
    defaultMinCuttingPower->setInputWidgetProperty("decimals", 1);
    defaultMinCuttingPower->setInputWidgetProperty("textTemplate", "%1%");
    defaultMinCuttingPower->setInputWidgetProperty("maximumLineEditWidth", 75);
    defaultMinCuttingPower->setInputWidgetProperty("page", 10);
    defaultMinCuttingPower->setInputWidgetProperty("minimum", 0);
    defaultMinCuttingPower->setInputWidgetProperty("maximum", 100);

    ConfigItem* defaultScanSpeed = group->addConfigItem(
        "defaultScanSpeed",
        400000,
        DT_INT
    );
    defaultScanSpeed->setInputWidgetType(IWT_FloatEditSlider);
    defaultScanSpeed->setInputWidgetProperty("step", 0.001);
    defaultScanSpeed->setInputWidgetProperty("decimals", 3);
    defaultScanSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    defaultScanSpeed->setInputWidgetProperty("textTemplate", "%1");
    defaultScanSpeed->setInputWidgetProperty("page", 10);
    defaultScanSpeed->setInputWidgetProperty("minimum", 1);
    defaultScanSpeed->setInputWidgetProperty("maximum", 2000);

    ConfigItem* maxScanGrayRatio = group->addConfigItem(
        "maxScanGrayRatio",
        950,
        DT_INT
    );
    maxScanGrayRatio->setInputWidgetProperty("maximumLineEditWidth", 75);
    maxScanGrayRatio->setInputWidgetProperty("minimum", 0);
    maxScanGrayRatio->setInputWidgetProperty("maximum", 1000);

    ConfigItem* minScanGrayRatio = group->addConfigItem(
        "minScanGrayRatio",
        0,
        DT_INT
    );
    minScanGrayRatio->setInputWidgetProperty("maximumLineEditWidth", 75);
    minScanGrayRatio->setInputWidgetProperty("minimum", 0);
    minScanGrayRatio->setInputWidgetProperty("maximum", 1000);

    ConfigItem* cuttingTurnOnDelay = group->addConfigItem(
        "cuttingTurnOnDelay",
        0.0,
        DT_REAL
    );
    cuttingTurnOnDelay->setInputWidgetProperty("minimum", 0.0);
    cuttingTurnOnDelay->setInputWidgetProperty("maximum", 10000.0);
    cuttingTurnOnDelay->setInputWidgetProperty("decimals", 3);

    ConfigItem* cuttingTurnOffDelay = group->addConfigItem(
        "cuttingTurnOffDelay",
        0.0,
        DT_REAL
    );
    cuttingTurnOffDelay->setInputWidgetProperty("minimum", 0.0);
    cuttingTurnOffDelay->setInputWidgetProperty("maximum", 10000.0);
    cuttingTurnOffDelay->setInputWidgetProperty("decimals", 3);

    ConfigItem* spotShotPower = group->addConfigItem(
        "spotShotPower",
        100,
        DT_INT
    );
    spotShotPower->setInputWidgetType(IWT_FloatEditSlider);
    spotShotPower->setInputWidgetProperty("step", 0.1);
    spotShotPower->setInputWidgetProperty("decimals", 1);
    spotShotPower->setInputWidgetProperty("textTemplate", "%1%");
    spotShotPower->setInputWidgetProperty("maximumLineEditWidth", 75);
    spotShotPower->setInputWidgetProperty("page", 10);
    spotShotPower->setInputWidgetProperty("minimum", 0);
    spotShotPower->setInputWidgetProperty("maximum", 100);

    ConfigItem* fillingSpeed = group->addConfigItem(
        "fillingSpeed",
        200000,
        DT_INT
    );
    fillingSpeed->setInputWidgetType(IWT_FloatEditSlider);
    fillingSpeed->setInputWidgetProperty("step", 0.001);
    fillingSpeed->setInputWidgetProperty("decimals", 3);
    fillingSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    fillingSpeed->setInputWidgetProperty("textTemplate", "%1");
    fillingSpeed->setInputWidgetProperty("page", 10);
    fillingSpeed->setInputWidgetProperty("minimum", 1);
    fillingSpeed->setInputWidgetProperty("maximum", 1000);

    ConfigItem* fillingStartSpeed = group->addConfigItem(
        "fillingStartSpeed",
        30000,
        DT_INT
    );
    fillingStartSpeed->setInputWidgetType(IWT_FloatEditSlider);
    fillingStartSpeed->setInputWidgetProperty("step", 0.001);
    fillingStartSpeed->setInputWidgetProperty("decimals", 3);
    fillingStartSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    fillingStartSpeed->setInputWidgetProperty("textTemplate", "%1");
    fillingStartSpeed->setInputWidgetProperty("page", 10);
    fillingStartSpeed->setInputWidgetProperty("minimum", 1);
    fillingStartSpeed->setInputWidgetProperty("maximum", 100);

    ConfigItem* fillingAcceleration = group->addConfigItem(
        "fillingAcceleration",
        3000000,
        DT_INT
    );
    fillingAcceleration->setInputWidgetType(IWT_FloatEditSlider);
    fillingAcceleration->setInputWidgetProperty("step", 0.001);
    fillingAcceleration->setInputWidgetProperty("decimals", 3);
    fillingAcceleration->setInputWidgetProperty("maximumLineEditWidth", 75);
    fillingAcceleration->setInputWidgetProperty("textTemplate", "%1");
    fillingAcceleration->setInputWidgetProperty("page", 10);
    fillingAcceleration->setInputWidgetProperty("minimum", 1);
    fillingAcceleration->setInputWidgetProperty("maximum", 10000);

    ConfigItem* maxFillingPower = group->addConfigItem(
        "maxFillingPower",
        124,
        DT_INT
    );
    maxFillingPower->setInputWidgetType(IWT_FloatEditSlider);
    maxFillingPower->setInputWidgetProperty("step", 0.1);
    maxFillingPower->setInputWidgetProperty("decimals", 1);
    maxFillingPower->setInputWidgetProperty("textTemplate", "%1%");
    maxFillingPower->setInputWidgetProperty("maximumLineEditWidth", 75);
    maxFillingPower->setInputWidgetProperty("page", 10);
    maxFillingPower->setInputWidgetProperty("minimum", 0);
    maxFillingPower->setInputWidgetProperty("maximum", 100);

    ConfigItem* minFillingPower = group->addConfigItem(
        "minFillingPower",
        97,
        DT_INT
    );
    minFillingPower->setInputWidgetType(IWT_FloatEditSlider);
    minFillingPower->setInputWidgetProperty("step", 0.1);
    minFillingPower->setInputWidgetProperty("decimals", 1);
    minFillingPower->setInputWidgetProperty("textTemplate", "%1%");
    minFillingPower->setInputWidgetProperty("maximumLineEditWidth", 75);
    minFillingPower->setInputWidgetProperty("page", 10);
    minFillingPower->setInputWidgetProperty("minimum", 0);
    minFillingPower->setInputWidgetProperty("maximum", 100);

    ConfigItem* fillingAccRatio = group->addConfigItem(
        "fillingAccRatio",
        100,
        DT_INT
    );
    fillingAccRatio->setInputWidgetProperty("maximumLineEditWidth", 75);
    fillingAccRatio->setInputWidgetProperty("minimum", 1);
    fillingAccRatio->setInputWidgetProperty("maximum", 200);
}

void Config::loadSystemRegisters()
{
    ConfigItemGroup* group = new Config::SystemRegister;
    group->setLazy(true);
    Config::SystemRegister::group = group;

    ConfigItem* head = group->addConfigItem(
        "head",
        0x12345678,
        DT_INT
    );
    head->setInputWidgetType(IWT_LineEdit);
    head->setInputWidgetProperty("readOnly", true);
    head->setVisible(false);

    ConfigItem* password = group->addConfigItem(
        "password",
        "",
        DT_STRING
    );
    password->setInputWidgetType(IWT_LineEdit);
    password->setVisible(false);

    ConfigItem* storedPassword = group->addConfigItem(
        "storedPassword",
        "",
        DT_STRING
    );
    storedPassword->setInputWidgetType(IWT_LineEdit);
    storedPassword->setVisible(false);

    ConfigItem* hardwareID1 = group->addConfigItem(
        "hardwareID1",
        "",
        DT_STRING
    );
    hardwareID1->setInputWidgetType(IWT_LineEdit);
    hardwareID1->setInputWidgetProperty("readOnly", true);
    hardwareID1->setVisible(false);

    ConfigItem* hardwareID2 = group->addConfigItem(
        "hardwareID2",
        "",
        DT_STRING
    );
    hardwareID2->setInputWidgetType(IWT_LineEdit);
    hardwareID2->setInputWidgetProperty("readOnly", true);
    hardwareID2->setVisible(false);

    ConfigItem* hardwareID3 = group->addConfigItem(
        "hardwareID3",
        "",
        DT_STRING
    );
    hardwareID3->setInputWidgetType(IWT_LineEdit);
    hardwareID3->setInputWidgetProperty("readOnly", true);
    hardwareID3->setVisible(false);

    ConfigItem* cdKey1 = group->addConfigItem(
        "cdKey1",
        "",
        DT_STRING
    );
    cdKey1->setInputWidgetType(IWT_LineEdit);
    cdKey1->setVisible(false);

    ConfigItem* cdKey2 = group->addConfigItem(
        "cdKey2",
        "",
        DT_STRING
    );
    cdKey2->setInputWidgetType(IWT_LineEdit);
    cdKey2->setVisible(false);

    ConfigItem* cdKey3 = group->addConfigItem(
        "cdKey3",
        "",
        DT_STRING
    );
    cdKey3->setInputWidgetType(IWT_LineEdit);
    cdKey3->setVisible(false);

    ConfigItem* sysRunTime = group->addConfigItem(
        "sysRunTime",
        0,
        DT_INT
    );
    sysRunTime->setInputWidgetType(IWT_LineEdit);

    ConfigItem* laserRunTime = group->addConfigItem(
        "laserRunTime",
        0,
        DT_INT
    );
    laserRunTime->setInputWidgetType(IWT_LineEdit);

    ConfigItem* sysRunNum = group->addConfigItem(
        "sysRunNum",
        0,
        DT_INT
    );
    sysRunNum->setInputWidgetType(IWT_LineEdit);

    ConfigItem* xMaxLength = group->addConfigItem(
        "xMaxLength",
        260000,
        DT_INT
    );
    xMaxLength->setInputWidgetType(IWT_FloatEditSlider);
    xMaxLength->setInputWidgetProperty("step", 0.001);
    xMaxLength->setInputWidgetProperty("decimals", 3);
    xMaxLength->setInputWidgetProperty("maximumLineEditWidth", 75);
    xMaxLength->setInputWidgetProperty("page", 10);
    xMaxLength->setInputWidgetProperty("minimum", 1);
    xMaxLength->setInputWidgetProperty("maximum", 3000);
    xMaxLength->setNeedRelaunch(true);

    ConfigItem* xDirPhase = group->addConfigItem(
        "xDirPhase",
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

            comboBox->addItem(tr("-"), 0);
            comboBox->addItem(tr("+"), 1);

            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );

    ConfigItem* xLimitPhase = group->addConfigItem(
        "xLimitPhase",
        0
    );
    xLimitPhase->setInputWidgetType(IWT_ComboBox);
    xLimitPhase->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item, InputWidgetWrapper* wrapper)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (!comboBox)
                return;

            comboBox->addItem(tr("-"), 0);
            comboBox->addItem(tr("+"), 1);

            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );

    ConfigItem* xZeroDev = group->addConfigItem(
        "xZeroDev",
        5000,
        DT_INT
    );
    xZeroDev->setInputWidgetType(IWT_FloatEditSlider);
    xZeroDev->setInputWidgetProperty("step", 0.001);
    xZeroDev->setInputWidgetProperty("decimals", 3);
    xZeroDev->setInputWidgetProperty("maximumLineEditWidth", 75);
    xZeroDev->setInputWidgetProperty("page", 1);
    xZeroDev->setInputWidgetProperty("minimum", 0);
    xZeroDev->setInputWidgetProperty("maximum", 3000);

    ConfigItem* xStepLength = group->addConfigItem(
        "xStepLength",
        6329114,
        DT_INT
    );
    xStepLength->setInputWidgetType(IWT_FloatEditSlider);
    xStepLength->setInputWidgetProperty("maximumLineEditWidth", 75);
    xStepLength->setInputWidgetProperty("step", 0.000001);
    xStepLength->setInputWidgetProperty("page", 0.001);
    xStepLength->setInputWidgetProperty("minimum", 0.000001);
    xStepLength->setInputWidgetProperty("maximum", 100);
    xStepLength->setInputWidgetProperty("decimals", 6);

    ConfigItem* xLimitNum = group->addConfigItem(
        "xLimitNum",
        0
    );
    xLimitNum->setInputWidgetType(IWT_ComboBox);
    xLimitNum->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item, InputWidgetWrapper* wrapper)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (!comboBox)
                return;

            comboBox->addItem(tr("x"), 0);
            comboBox->addItem(tr("y"), 1);
            comboBox->addItem(tr("z"), 1);
            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );

    ConfigItem* xResetEnabled = group->addConfigItem(
        "xResetEnabled",
        true,
        DT_INT
    );
    xResetEnabled->setInputWidgetType(IWT_CheckBox);

    ConfigItem* xMotorNum = group->addConfigItem(
        "xMotorNum",
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

            comboBox->addItem(tr("x"), 0);
            comboBox->addItem(tr("y"), 1);
            comboBox->addItem(tr("z"), 1);
            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );

    ConfigItem* xMotorCurrent = group->addConfigItem(
        "xMotorCurrent",
        60,
        DT_INT
    );
    xMotorCurrent->setInputWidgetProperty("textTemplate", "%1%");
    xMotorCurrent->setInputWidgetProperty("maximumLineEditWidth", 75);
    xMotorCurrent->setInputWidgetProperty("step", 1);
    xMotorCurrent->setInputWidgetProperty("page", 10);
    xMotorCurrent->setInputWidgetProperty("minimum", 20);
    xMotorCurrent->setInputWidgetProperty("maximum", 100);

    ConfigItem* xStartSpeed = group->addConfigItem(
        "xStartSpeed",
        30000,
        DT_INT
    );
    xStartSpeed->setInputWidgetType(IWT_FloatEditSlider);
    xStartSpeed->setInputWidgetProperty("step", 0.001);
    xStartSpeed->setInputWidgetProperty("decimals", 3);
    xStartSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    xStartSpeed->setInputWidgetProperty("page", 10);
    xStartSpeed->setInputWidgetProperty("minimum", 1);
    xStartSpeed->setInputWidgetProperty("maximum", 100);

    ConfigItem* xMaxSpeed = group->addConfigItem(
        "xMaxSpeed",
        600000,
        DT_INT
    );
    xMaxSpeed->setInputWidgetType(IWT_FloatEditSlider);
    xMaxSpeed->setInputWidgetProperty("step", 0.001);
    xMaxSpeed->setInputWidgetProperty("decimals", 3);
    xMaxSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    xMaxSpeed->setInputWidgetProperty("page", 10);
    xMaxSpeed->setInputWidgetProperty("minimum", 1);
    xMaxSpeed->setInputWidgetProperty("maximum", 2000);

    ConfigItem* xMaxAcceleration = group->addConfigItem(
        "xMaxAcceleration",
        8000000,
        DT_INT
    );
    xMaxAcceleration->setInputWidgetType(IWT_FloatEditSlider);
    xMaxAcceleration->setInputWidgetProperty("step", 0.001);
    xMaxAcceleration->setInputWidgetProperty("decimals", 3);
    xMaxAcceleration->setInputWidgetProperty("maximumLineEditWidth", 75);
    xMaxAcceleration->setInputWidgetProperty("page", 10);
    xMaxAcceleration->setInputWidgetProperty("minimum", 1);
    xMaxAcceleration->setInputWidgetProperty("maximum", 20000);

    ConfigItem* xUrgentAcceleration = group->addConfigItem(
        "xUrgentAcceleration",
        4000000,
        DT_INT
    );
    xUrgentAcceleration->setInputWidgetType(IWT_FloatEditSlider);
    xUrgentAcceleration->setInputWidgetProperty("step", 0.001);
    xUrgentAcceleration->setInputWidgetProperty("decimals", 3);
    xUrgentAcceleration->setInputWidgetProperty("maximumLineEditWidth", 75);
    xUrgentAcceleration->setInputWidgetProperty("page", 10);
    xUrgentAcceleration->setInputWidgetProperty("minimum", 1);
    xUrgentAcceleration->setInputWidgetProperty("maximum", 20000);

    ConfigItem* yMaxLength = group->addConfigItem(
        "yMaxLength",
        200000,
        DT_INT
    );
    yMaxLength->setInputWidgetType(IWT_FloatEditSlider);
    yMaxLength->setInputWidgetProperty("step", 0.001);
    yMaxLength->setInputWidgetProperty("decimals", 3);
    yMaxLength->setInputWidgetProperty("maximumLineEditWidth", 75);
    yMaxLength->setInputWidgetProperty("page", 1);
    yMaxLength->setInputWidgetProperty("minimum", 1);
    yMaxLength->setInputWidgetProperty("maximum", 5000);
    yMaxLength->setNeedRelaunch(true);

    ConfigItem* yDirPhase = group->addConfigItem(
        "yDirPhase",
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

            comboBox->addItem(tr("-"), 0);
            comboBox->addItem(tr("+"), 1);
            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );

    ConfigItem* yLimitPhase = group->addConfigItem(
        "yLimitPhase",
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

            comboBox->addItem(tr("-"), 0);
            comboBox->addItem(tr("+"), 1);
            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );

    ConfigItem* yZeroDev = group->addConfigItem(
        "yZeroDev",
        5000,
        DT_INT
    );
    yZeroDev->setInputWidgetType(IWT_FloatEditSlider);
    yZeroDev->setInputWidgetProperty("step", 0.001);
    yZeroDev->setInputWidgetProperty("decimals", 3);
    yZeroDev->setInputWidgetProperty("maximumLineEditWidth", 75);
    yZeroDev->setInputWidgetProperty("page", 1);
    yZeroDev->setInputWidgetProperty("minimum", 0);
    yZeroDev->setInputWidgetProperty("maximum", 5000);

    ConfigItem* yStepLength = group->addConfigItem(
        "yStepLength",
        6329114,
        DT_INT
    );
    yStepLength->setInputWidgetType(IWT_FloatEditSlider);
    yStepLength->setInputWidgetProperty("maximumLineEditWidth", 75);
    yStepLength->setInputWidgetProperty("step", 0.000001);
    yStepLength->setInputWidgetProperty("page", 0.001);
    yStepLength->setInputWidgetProperty("minimum", 0.000001);
    yStepLength->setInputWidgetProperty("maximum", 100);
    yStepLength->setInputWidgetProperty("decimals", 6);

    ConfigItem* yLimitNum = group->addConfigItem(
        "yLimitNum",
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

            comboBox->addItem(tr("x"), 0);
            comboBox->addItem(tr("y"), 1);
            comboBox->addItem(tr("z"), 1);
            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );

    ConfigItem* yResetEnabled = group->addConfigItem(
        "yResetEnabled",
        true,
        DT_INT
    );
    yResetEnabled->setInputWidgetType(IWT_CheckBox);

    ConfigItem* yMotorNum = group->addConfigItem(
        "yMotorNum",
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

            comboBox->addItem(tr("x"), 0);
            comboBox->addItem(tr("y"), 1);
            comboBox->addItem(tr("z"), 1);
            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );

    ConfigItem* yMotorCurrent = group->addConfigItem(
        "yMotorCurrent",
        60,
        DT_INT
    );
    yMotorCurrent->setInputWidgetProperty("textTemplate", "%1%");
    yMotorCurrent->setInputWidgetProperty("maximumLineEditWidth", 75);
    yMotorCurrent->setInputWidgetProperty("step", 1);
    yMotorCurrent->setInputWidgetProperty("page", 10);
    yMotorCurrent->setInputWidgetProperty("minimum", 20);
    yMotorCurrent->setInputWidgetProperty("maximum", 100);

    ConfigItem* yStartSpeed = group->addConfigItem(
        "yStartSpeed",
        15000,
        DT_INT
    );
    yStartSpeed->setInputWidgetType(IWT_FloatEditSlider);
    yStartSpeed->setInputWidgetProperty("step", 0.001);
    yStartSpeed->setInputWidgetProperty("decimals", 3);
    yStartSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    yStartSpeed->setInputWidgetProperty("page", 10);
    yStartSpeed->setInputWidgetProperty("minimum", 1);
    yStartSpeed->setInputWidgetProperty("maximum", 100);

    ConfigItem* yMaxSpeed = group->addConfigItem(
        "yMaxSpeed",
        500000,
        DT_INT
    );
    yMaxSpeed->setInputWidgetType(IWT_FloatEditSlider);
    yMaxSpeed->setInputWidgetProperty("step", 0.001);
    yMaxSpeed->setInputWidgetProperty("decimals", 3);
    yMaxSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    yMaxSpeed->setInputWidgetProperty("page", 10);
    yMaxSpeed->setInputWidgetProperty("minimum", 1);
    yMaxSpeed->setInputWidgetProperty("maximum", 1000);

    ConfigItem* yMaxAcceleration = group->addConfigItem(
        "yMaxAcceleration",
        4000000,
        DT_INT
    );
    yMaxAcceleration->setInputWidgetType(IWT_FloatEditSlider);
    yMaxAcceleration->setInputWidgetProperty("step", 0.001);
    yMaxAcceleration->setInputWidgetProperty("decimals", 3);
    yMaxAcceleration->setInputWidgetProperty("maximumLineEditWidth", 75);
    yMaxAcceleration->setInputWidgetProperty("page", 10);
    yMaxAcceleration->setInputWidgetProperty("minimum", 1);
    yMaxAcceleration->setInputWidgetProperty("maximum", 20000);

    ConfigItem* yUrgentAcceleration = group->addConfigItem(
        "yUrgentAcceleration",
        3000000,
        DT_INT
    );
    yUrgentAcceleration->setInputWidgetType(IWT_FloatEditSlider);
    yUrgentAcceleration->setInputWidgetProperty("step", 0.001);
    yUrgentAcceleration->setInputWidgetProperty("decimals", 3);
    yUrgentAcceleration->setInputWidgetProperty("maximumLineEditWidth", 75);
    yUrgentAcceleration->setInputWidgetProperty("page", 10);
    yUrgentAcceleration->setInputWidgetProperty("minimum", 1);
    yUrgentAcceleration->setInputWidgetProperty("maximum", 20000);

    ConfigItem* zMaxLength = group->addConfigItem(
        "zMaxLength",
        200000,
        DT_INT
    );
    zMaxLength->setInputWidgetType(IWT_FloatEditSlider);
    zMaxLength->setInputWidgetProperty("step", 0.001);
    zMaxLength->setInputWidgetProperty("decimals", 3);
    zMaxLength->setInputWidgetProperty("maximumLineEditWidth", 75);
    zMaxLength->setInputWidgetProperty("page", 1);
    zMaxLength->setInputWidgetProperty("minimum", 1);
    zMaxLength->setInputWidgetProperty("maximum", 2000);
    zMaxLength->setNeedRelaunch(true);

    ConfigItem* zDirPhase = group->addConfigItem(
        "zDirPhase",
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

            comboBox->addItem(tr("-"), 0);
            comboBox->addItem(tr("+"), 1);
            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );

    ConfigItem* zLimitPhase = group->addConfigItem(
        "zLimitPhase",
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
            comboBox->addItem(tr("-"), 0);
            comboBox->addItem(tr("+"), 1);
            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
            comboBox->blockSignals(false);
        }
    );

    ConfigItem* zZeroDev = group->addConfigItem(
        "zZeroDev",
        0,
        DT_INT
    );
    zZeroDev->setInputWidgetType(IWT_FloatEditSlider);
    zZeroDev->setInputWidgetProperty("step", 0.001);
    zZeroDev->setInputWidgetProperty("decimals", 3);
    zZeroDev->setInputWidgetProperty("maximumLineEditWidth", 75);
    zZeroDev->setInputWidgetProperty("page", 1);
    zZeroDev->setInputWidgetProperty("minimum", 0);
    zZeroDev->setInputWidgetProperty("maximum", 2000);

    ConfigItem* zStepLength = group->addConfigItem(
        "zStepLength",
        6329114,
        DT_INT
    );
    zStepLength->setInputWidgetType(IWT_FloatEditSlider);
    zStepLength->setInputWidgetProperty("maximumLineEditWidth", 75);
    zStepLength->setInputWidgetProperty("step", 0.000001);
    zStepLength->setInputWidgetProperty("page", 0.001);
    zStepLength->setInputWidgetProperty("minimum", 0.000001);
    zStepLength->setInputWidgetProperty("maximum", 100);
    zStepLength->setInputWidgetProperty("decimals", 6);

    ConfigItem* zLimitNum = group->addConfigItem(
        "zLimitNum",
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

            comboBox->addItem(tr("x"), 0);
            comboBox->addItem(tr("y"), 1);
            comboBox->addItem(tr("z"), 2);
            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );

    ConfigItem* zResetEnabled = group->addConfigItem(
        "zResetEnabled",
        true,
        DT_INT
    );
    zResetEnabled->setInputWidgetType(IWT_CheckBox);

    ConfigItem* zMotorNum = group->addConfigItem(
        "zMotorNum",
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

            comboBox->addItem(tr("x"), 0);
            comboBox->addItem(tr("y"), 1);
            comboBox->addItem(tr("z"), 2);
            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );

    ConfigItem* zMotorCurrent = group->addConfigItem(
        "zMotorCurrent",
        60,
        DT_INT
    );
    zMotorCurrent->setInputWidgetProperty("textTemplate", "%1%");
    zMotorCurrent->setInputWidgetProperty("maximumLineEditWidth", 75);
    zMotorCurrent->setInputWidgetProperty("step", 1);
    zMotorCurrent->setInputWidgetProperty("page", 10);
    zMotorCurrent->setInputWidgetProperty("minimum", 20);
    zMotorCurrent->setInputWidgetProperty("maximum", 100);

    ConfigItem* zStartSpeed = group->addConfigItem(
        "zStartSpeed",
        1000,
        DT_INT
    );
    zStartSpeed->setInputWidgetType(IWT_FloatEditSlider);
    zStartSpeed->setInputWidgetProperty("step", 0.001);
    zStartSpeed->setInputWidgetProperty("decimals", 3);
    zStartSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    zStartSpeed->setInputWidgetProperty("page", 10);
    zStartSpeed->setInputWidgetProperty("minimum", 1);
    zStartSpeed->setInputWidgetProperty("maximum", 100);

    ConfigItem* zMaxSpeed = group->addConfigItem(
        "zMaxSpeed",
        5000,
        DT_INT
    );
    zMaxSpeed->setInputWidgetType(IWT_FloatEditSlider);
    zMaxSpeed->setInputWidgetProperty("step", 0.001);
    zMaxSpeed->setInputWidgetProperty("decimals", 3);
    zMaxSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    zMaxSpeed->setInputWidgetProperty("step", 0.001);
    zMaxSpeed->setInputWidgetProperty("page", 10);
    zMaxSpeed->setInputWidgetProperty("minimum", 1);
    zMaxSpeed->setInputWidgetProperty("maximum", 100);

    ConfigItem* zMaxAcceleration = group->addConfigItem(
        "zMaxAcceleration",
        500000,
        DT_INT
    );
    zMaxAcceleration->setInputWidgetType(IWT_FloatEditSlider);
    zMaxAcceleration->setInputWidgetProperty("step", 0.001);
    zMaxAcceleration->setInputWidgetProperty("decimals", 3);
    zMaxAcceleration->setInputWidgetProperty("maximumLineEditWidth", 75);
    zMaxAcceleration->setInputWidgetProperty("page", 10);
    zMaxAcceleration->setInputWidgetProperty("minimum", 1);
    zMaxAcceleration->setInputWidgetProperty("maximum", 20000);

    ConfigItem* zUrgentAcceleration = group->addConfigItem(
        "zUrgentAcceleration",
        500000,
        DT_INT
    );
    zUrgentAcceleration->setInputWidgetType(IWT_FloatEditSlider);
    zUrgentAcceleration->setInputWidgetProperty("step", 0.001);
    zUrgentAcceleration->setInputWidgetProperty("decimals", 3);
    zUrgentAcceleration->setInputWidgetProperty("maximumLineEditWidth", 75);
    zUrgentAcceleration->setInputWidgetProperty("step", 0.001);
    zUrgentAcceleration->setInputWidgetProperty("page", 10);
    zUrgentAcceleration->setInputWidgetProperty("minimum", 1);
    zUrgentAcceleration->setInputWidgetProperty("maximum", 20000);

    ConfigItem* laserMaxPower = group->addConfigItem(
        "laserMaxPower",
        1000,
        DT_INT
    );
    laserMaxPower->setInputWidgetType(IWT_FloatEditSlider);
    laserMaxPower->setInputWidgetProperty("step", 0.1);
    laserMaxPower->setInputWidgetProperty("decimals", 1);
    laserMaxPower->setInputWidgetProperty("textTemplate", "%1%");
    laserMaxPower->setInputWidgetProperty("maximumLineEditWidth", 75);
    laserMaxPower->setInputWidgetProperty("minimum", 1);
    laserMaxPower->setInputWidgetProperty("maximum", 100);

    ConfigItem* laserMinPower = group->addConfigItem(
        "laserMinPower",
        100,
        DT_INT
    );
    laserMinPower->setInputWidgetType(IWT_FloatEditSlider);
    laserMinPower->setInputWidgetProperty("step", 0.1);
    laserMinPower->setInputWidgetProperty("decimals", 1);
    laserMinPower->setInputWidgetProperty("textTemplate", "%1%");
    laserMinPower->setInputWidgetProperty("maximumLineEditWidth", 75);
    laserMinPower->setInputWidgetProperty("minimum", 1);
    laserMinPower->setInputWidgetProperty("maximum", 100);

    ConfigItem* laserPowerFreq = group->addConfigItem(
        "laserPowerFreq",
        4000
    );
    laserPowerFreq->setInputWidgetProperty("maximumLineEditWidth", 75);
    laserPowerFreq->setInputWidgetProperty("page", 1000);
    laserPowerFreq->setInputWidgetProperty("minimum", 1);
    laserPowerFreq->setInputWidgetProperty("maximum", 10000);
    laserPowerFreq->setVisible(false);

    ConfigItem* xPhaseEnabled = group->addConfigItem(
        "xPhaseEnabled",
        true,
        DT_INT
    );
    xPhaseEnabled->setInputWidgetType(IWT_CheckBox);

    ConfigItem* yPhaseEnabled = group->addConfigItem(
        "yPhaseEnabled",
        true,
        DT_INT
    );
    yPhaseEnabled->setInputWidgetType(IWT_CheckBox);

    ConfigItem* zPhaseEnabled = group->addConfigItem(
        "zPhaseEnabled",
        true,
        DT_INT
    );
    zPhaseEnabled->setInputWidgetType(IWT_CheckBox);

    ConfigItem* deviceOrigin = group->addConfigItem(
        "deviceOrigin",
        0,
        DT_INT
    );
    deviceOrigin->setInputWidgetType(IWT_Custom);
    deviceOrigin->setCreateWidgetHook(
        [=](ConfigItem* item) {
            RadioButtonGroup* widget = new RadioButtonGroup(2, 2);
            widget->setValues(QList<int>() << 0 << 3 << 1 << 2);
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
            return value;
        }
    );
    deviceOrigin->setNeedRelaunch(true);

    ConfigItem* zResetSpeed = group->addConfigItem(
        "zResetSpeed",
        50000,
        DT_INT
    );
    zResetSpeed->setInputWidgetType(IWT_FloatEditSlider);
    zResetSpeed->setInputWidgetProperty("step", 0.001);
    zResetSpeed->setInputWidgetProperty("decimals", 0);
    zResetSpeed->setInputWidgetProperty("maximumLineEditWidth", 75);
    zResetSpeed->setInputWidgetProperty("textTemplate", "%1");
    zResetSpeed->setInputWidgetProperty("page", 10);
    zResetSpeed->setInputWidgetProperty("minimum", 1);
    zResetSpeed->setInputWidgetProperty("maximum", 100);
}

void Config::loadDebug()
{
    ConfigItemGroup* group = new Config::Debug;
    Config::Debug::group = group;

    ConfigItem* showPrimitiveName = group->addConfigItem(
        "showPrimitiveName",
        false,
        DT_BOOL
    );

    ConfigItem* showPrimitiveFirstPoint = group->addConfigItem(
        "showPrimitiveFirstPoint",
        false,
        DT_BOOL
    );

    ConfigItem* generatePathImage = group->addConfigItem(
        "generatePathImage",
        false,
        DT_BOOL
    );

    ConfigItem* generateMachiningImage = group->addConfigItem(
        "generateMachiningImage",
        false,
        DT_BOOL
    );

    ConfigItem* enableOptimizeInteraction = group->addConfigItem(
        "enableOptimizeInteraction",
        false,
        DT_BOOL
    );
}

QList<ConfigItemGroup*> Config::getGroups()
{
    return groups;
}

#define TRANS_TITLE_DESC(groupName, itemName, title, desc) \
    groupName::itemName##Item()->setTitle(QCoreApplication::translate("Config", title, nullptr)); \
    groupName::itemName##Item()->setDescription(QCoreApplication::translate("Config", desc, nullptr));
    
void Config::updateTitlesAndDescriptions()
{
    General::languageItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Language", nullptr), 
        QCoreApplication::translate("Config", "Language for both UI and Business.", nullptr));

    General::unitItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Unit", nullptr), 
        QCoreApplication::translate("Config", "Unit for user interface.", nullptr));

    Layers::maxLayersCountItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Max Layers Count", nullptr), 
        QCoreApplication::translate("Config", "Max Layers Count", nullptr));

    Ui::operationButtonIconSizeItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Operation Button Icon Size(px)", nullptr), 
        QCoreApplication::translate("Config", "Size of operation buttons' icons", nullptr));

    Ui::operationButtonWidthItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Operation Button Width(px)", nullptr), 
        QCoreApplication::translate("Config", "Width of operation buttons", nullptr));

    Ui::operationButtonHeightItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Operation Button Height(px)", nullptr), 
        QCoreApplication::translate("Config", "Height of operation buttons", nullptr));

    Ui::operationButtonShowTextItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Show Operation Button Text", nullptr), 
        QCoreApplication::translate("Config", "Show text of operation button or not", nullptr));

    Ui::toolButtonSizeItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Tool Button Size(px)", nullptr), 
        QCoreApplication::translate("Config", "Size of tool buttons", nullptr));

    Ui::colorButtonWidthItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Color Button Width(px)", nullptr), 
        QCoreApplication::translate("Config", "Width of color buttons", nullptr));

    Ui::colorButtonHeightItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Color Button Height(px)", nullptr), 
        QCoreApplication::translate("Config", "Height of color buttons", nullptr));

    Ui::gridContrastItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Grid Contrast", nullptr), 
        QCoreApplication::translate("Config", "Contrast of grid lines", nullptr));

    Ui::gridShapeDistanceItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Grid Shape Distance(px)", nullptr), 
        QCoreApplication::translate("Config", "This distance is used for capturing a shape when cursor is moving closed to", nullptr));

    Ui::objectShapeDistanceItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Object Shape Distance(px)", nullptr), 
        QCoreApplication::translate("Config", "This distance is used for capturing an object when cursor is moving closed to", nullptr));

    Ui::clickSelectionToleranceItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Click-selection Tolerance(px)", nullptr), 
        QCoreApplication::translate("Config", "Tolerance of Click-selection in px", nullptr));

    Ui::visualGridSpacingItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Visual Grid Spacing(mm)", nullptr), 
        QCoreApplication::translate("Config", "The visual grid spacing in mm", nullptr));

    Ui::validMaxRegionItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Valid Max Region(mm)", nullptr),
        QCoreApplication::translate("Config", "The Valid Max Region Width Or Height in mm", nullptr));

    Ui::splitterHandleWidthItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Splitter Handle Width(px)", nullptr), 
        QCoreApplication::translate("Config", "Width of splitter handle in px", nullptr));

    Ui::autoRepeatDelayItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Auto repeat delay(ms)", nullptr), 
        QCoreApplication::translate("Config", "The delay duration of auto repeat button", nullptr));

    Ui::showDocumentBoundingRectItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Show Document Bounding Rect", nullptr), 
        QCoreApplication::translate("Config", "Show document bounding rect", nullptr));

    Ui::laserCursorTimeoutItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Show Laser Cursor Timeout", nullptr), 
        QCoreApplication::translate("Config", "Show laser cursor timeout", nullptr));

    CuttingLayer::runSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Cutting Speed(mm)", nullptr), 
        QCoreApplication::translate("Config", "Cutting speed for cutting layers", nullptr));

    CuttingLayer::minPowerItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Cutting Min Power(%)", nullptr), 
        QCoreApplication::translate("Config", "The min power percentage for cutting layers", nullptr));

    CuttingLayer::maxPowerItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Cutting Max Power(%)", nullptr), 
        QCoreApplication::translate("Config", "The max power percentage for cutting layers", nullptr));

    EngravingLayer::runSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Engraving Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "The run speed for engraving layers", nullptr));

    EngravingLayer::laserPowerItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Engraving Power(%)", nullptr), 
        QCoreApplication::translate("Config", "The laser power for engraving layers", nullptr));

    EngravingLayer::minPowerItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Engraving Min Power(%)", nullptr), 
        QCoreApplication::translate("Config", "The min power percentage for engraving layers", nullptr));

    EngravingLayer::maxPowerItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Engraving Max Power(%)", nullptr), 
        QCoreApplication::translate("Config", "The max power percentage for engraving layers", nullptr));

    EngravingLayer::rowIntervalItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Row Interval(μm)", nullptr), 
        QCoreApplication::translate("Config", "The row interval between lines of bitmap for engraving layers", nullptr));

    EngravingLayer::useHalftoneItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Use Halftone", nullptr), 
        QCoreApplication::translate("Config", "Use halftone algorithm when generating bitmap datas", nullptr));

    EngravingLayer::halftoneAnglesItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Halftone Angles", nullptr), 
        QCoreApplication::translate("Config", "Angles of rotating grid for halftone", nullptr));

    EngravingLayer::halftoneGridSizeItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Halftone Grid Size", nullptr), 
        QCoreApplication::translate("Config", "Grid size of halftone", nullptr));

    EngravingLayer::useHalftoneItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Use Halftone", nullptr), 
        QCoreApplication::translate("Config", "Use halftone algorithm when generating bitmap datas", nullptr));

    EngravingLayer::LPIItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "LPI", nullptr), 
        QCoreApplication::translate("Config", "Lines per inch", nullptr));

    EngravingLayer::DPIItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "DPI", nullptr), 
        QCoreApplication::translate("Config", "Dots per inch", nullptr));

    EngravingLayer::enableCuttingItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Enable Cutting Item", nullptr), 
        QCoreApplication::translate("Config", "Enable Cutting Item", nullptr));

    FillingLayer::runSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Filling Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "The run speed for filling layers", nullptr));

    FillingLayer::minPowerItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Filling Min Power(%)", nullptr), 
        QCoreApplication::translate("Config", "The min power percentage for filling layers", nullptr));

    FillingLayer::maxPowerItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Filling Max Power(%)", nullptr), 
        QCoreApplication::translate("Config", "The max power percentage for filling layers", nullptr));

    FillingLayer::rowIntervalItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Row Interval(μm)", nullptr), 
        QCoreApplication::translate("Config", "The row interval between lines of bitmap for filling layers", nullptr));

    FillingLayer::enableCuttingItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Enable Cutting Item", nullptr), 
        QCoreApplication::translate("Config", "Enable Cutting Item", nullptr));

    FillingLayer::fillingTypeItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Filling Type", nullptr), 
        QCoreApplication::translate("Config", "Filling Type", nullptr));

    PathOptimization::maxStartingPointsItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Max Starting Points", nullptr), 
        QCoreApplication::translate("Config", "Max starting points count of each primitive node", nullptr));

    PathOptimization::groupingOrientationItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Grouping Orientation", nullptr), 
        QCoreApplication::translate("Config", "The orientation of grouping", nullptr));

    PathOptimization::groupingGridIntervalItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Grouping Grid Interval", nullptr), 
        QCoreApplication::translate("Config", "The grid interval", nullptr));

    PathOptimization::maxGroupSizeItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Max Group size", nullptr), 
        QCoreApplication::translate("Config", "Max nodes count of each group", nullptr));

    PathOptimization::searchingXYWeightItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Searching XY Weight", nullptr), 
        QCoreApplication::translate("Config", "Weight of XY for searching using kdtree", nullptr));

    PathOptimization::searchingXYWeightItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Searching XY Weight", nullptr), 
        QCoreApplication::translate("Config", "Weight of XY for searching using kdtree", nullptr));

    Export::maxAnglesDiffItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Max Angles Diff", nullptr), 
        QCoreApplication::translate("Config", "The max angles diff bwteen tow anchor points", nullptr));

    Export::maxIntervalDistanceItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Max Interval Distance", nullptr), 
        QCoreApplication::translate("Config", "The max interval distance between tow anchor points", nullptr));

    Export::maxIntervalDistanceItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Max Interval Distance", nullptr), 
        QCoreApplication::translate("Config", "The max interval distance between tow anchor points", nullptr));

    Export::enableSmallDiagonalItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Enable Small Diagonal", nullptr), 
        QCoreApplication::translate("Config", "Enable small diagonal limitation for small primitives", nullptr));

    Export::smallDiagonalLimitationItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Small Diagonal Limitation", nullptr), 
        QCoreApplication::translate("Config", "Details of small diagonal limitation", nullptr));

    Export::curveFlatteningThresholdItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Curve Flattening Threshold", nullptr), 
        QCoreApplication::translate("Config", "Curve Flattening Threshold", nullptr));

    Export::gaussianFactorAItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Gaussian Factor A", nullptr), 
        QCoreApplication::translate("Config", "Gaussian factor a", nullptr));

    Device::autoConnectFirstCOMItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Auto Connect First COM", nullptr), 
        QCoreApplication::translate("Config", "Auto connect to first com port when found multiple laser devices", nullptr));

    Device::startFromItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Start from", nullptr), 
        QCoreApplication::translate("Config", "Choose the start point type of machining", nullptr));

    Device::jobOriginItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Job Origin", nullptr), 
        QCoreApplication::translate("Config", "Job origin to start machining with", nullptr));

    Device::xEnabledItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "X Enabled", nullptr), 
        QCoreApplication::translate("Config", "Enabled x axis movement", nullptr));

    Device::yEnabledItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Y Enabled", nullptr), 
        QCoreApplication::translate("Config", "Enabled y axis movement", nullptr));

    Device::zEnabledItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Z Enabled", nullptr), 
        QCoreApplication::translate("Config", "Enabled z axis movement", nullptr));

    Device::userOrigin1Item()->setTitleAndDesc(
        QCoreApplication::translate("Config", "User Origin 1", nullptr), 
        QCoreApplication::translate("Config", "User Origin 1", nullptr));

    Device::userOrigin2Item()->setTitleAndDesc(
        QCoreApplication::translate("Config", "User Origin 2", nullptr), 
        QCoreApplication::translate("Config", "User Origin 2", nullptr));

    Device::userOrigin3Item()->setTitleAndDesc(
        QCoreApplication::translate("Config", "User Origin 3", nullptr), 
        QCoreApplication::translate("Config", "User Origin 3", nullptr));

    Device::userOriginSelectedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "User Origin", nullptr), 
        QCoreApplication::translate("Config", "Selected user origin", nullptr));

    Device::zReverseDirectionItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Reverse Z Direction", nullptr), 
        QCoreApplication::translate("Config", "Reverse Z Direction", nullptr));

    Device::zFocalLengthItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Z Focal Length(mm)", nullptr), 
        QCoreApplication::translate("Config", "Z Focal Length", nullptr));

    Device::calibrationBlockThicknessItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Calibration block thickness(mm)", nullptr), 
        QCoreApplication::translate("Config", "Calibration block thickness", nullptr));

    UserRegister::headItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Head Data", nullptr), 
        QCoreApplication::translate("Config", "Read-only data used to test the quality of data transmission", nullptr));

    UserRegister::accModeItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Acceleration Mode", nullptr), 
        QCoreApplication::translate("Config", "Acceleration mode", nullptr));

    UserRegister::cuttingMoveSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Cutting Move Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "Acceleration mode", nullptr));

    UserRegister::cuttingMoveAccItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Cutting Move Acceleration(mm/s<sup>2</sup>)", nullptr), 
        QCoreApplication::translate("Config", "Move acceleration for cutting movement", nullptr));

    UserRegister::cuttingTurnSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Cutting Turn Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "Turn speed for cutting movement", nullptr));

    UserRegister::cuttingTurnAccItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Cutting Turn Acceleration(mm/s<sup>2</sup>)", nullptr), 
        QCoreApplication::translate("Config", "Turn acceleration for cutting movement", nullptr));

    UserRegister::cuttingWorkAccItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Cutting Work Acceleration(mm/s<sup>2</sup>)", nullptr), 
        QCoreApplication::translate("Config", "Work acceleration for cutting movement", nullptr));

    UserRegister::cuttingMoveSpeedFactorItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Move Speed Factor", nullptr), 
        QCoreApplication::translate("Config", "Move speed factor for cutting movement", nullptr));

    UserRegister::cuttingWorkSpeedFactorItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Work Speed Factor", nullptr), 
        QCoreApplication::translate("Config", "Work speed factor for cutting movement", nullptr));

    UserRegister::cuttingSpotSizeItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Cutting Spot Size", nullptr), 
        QCoreApplication::translate("Config", "Spot size for cutting movement", nullptr));

    UserRegister::scanXStartSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Scan X Start Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "Scan x start speed", nullptr));

    UserRegister::scanYStartSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Scan Y Start Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "Scan y start speed", nullptr));

    UserRegister::scanXAccItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Scan X Acceleration(mm/s<sup>2</sup>)", nullptr), 
        QCoreApplication::translate("Config", "Scan x acceleration", nullptr));

    UserRegister::scanYAccItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Scan Y Acceleration(mm/s<sup>2</sup>)", nullptr), 
        QCoreApplication::translate("Config", "Scan y acceleration", nullptr));

    UserRegister::scanRowSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Scan Row Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "Scan row speed", nullptr));

    UserRegister::scanLaserFrequencyItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Laser Frequency", nullptr), 
        QCoreApplication::translate("Config", "Laser Frequency", nullptr));

    UserRegister::scanReturnErrorItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Scan Return Error(mm)", nullptr), 
        QCoreApplication::translate("Config", "Scan return error", nullptr));

    UserRegister::scanLaserPowerItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Scan Laser Power(%)", nullptr), 
        QCoreApplication::translate("Config", "The percentage of scan laser power", nullptr));

    UserRegister::scanXResetEnabledItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Scan X Reset Enabled", nullptr), 
        QCoreApplication::translate("Config", "Enabled Scan X reset", nullptr));

    UserRegister::scanYResetEnabledItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Scan Y Reset Enabled", nullptr), 
        QCoreApplication::translate("Config", "Enabled Scan Y reset", nullptr));

    UserRegister::scanZResetEnabledItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Scan Z Reset Enabled", nullptr), 
        QCoreApplication::translate("Config", "Enabled Scan Z reset", nullptr));

    UserRegister::resetSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Reset Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "Reset speed", nullptr));

    UserRegister::scanReturnPosItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Scan Return Pos", nullptr), 
        QCoreApplication::translate("Config", "Scan return pos", nullptr));

    UserRegister::backlashXIntervalItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Backlash X Interval(mm)", nullptr), 
        QCoreApplication::translate("Config", "Backlash X interval", nullptr));

    UserRegister::backlashYIntervalItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Backlash Y Interval(mm)", nullptr), 
        QCoreApplication::translate("Config", "Backlash Y interval", nullptr));

    UserRegister::backlashZIntervalItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Backlash Z Interval(mm)", nullptr), 
        QCoreApplication::translate("Config", "Backlash Z interval", nullptr));

    UserRegister::defaultRunSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Default Run Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "Default run speed", nullptr));

    UserRegister::defaultMaxCuttingPowerItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Default Max Cutting Power(%)", nullptr), 
        QCoreApplication::translate("Config", "Default max cutting power", nullptr));

    UserRegister::defaultMinCuttingPowerItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Default Min Cutting Power(%)", nullptr), 
        QCoreApplication::translate("Config", "Default min cutting power", nullptr));

    UserRegister::defaultScanSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Default Scan Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "Default scan speed", nullptr));

    UserRegister::maxScanGrayRatioItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Max Scan Gray Ratio", nullptr), 
        QCoreApplication::translate("Config", "Max scan gray ratio", nullptr));

    UserRegister::minScanGrayRatioItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Min Scan Gray Ratio", nullptr), 
        QCoreApplication::translate("Config", "Min scan gray ratio", nullptr));

    UserRegister::cuttingTurnOnDelayItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Cutting Turn On Delay(us)", nullptr), 
        QCoreApplication::translate("Config", "Delay of turning on laser for cutting", nullptr));

    UserRegister::cuttingTurnOffDelayItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Cutting Turn Off Delay(us)", nullptr), 
        QCoreApplication::translate("Config", "Delay of turning off laser for cutting", nullptr));

    UserRegister::spotShotPowerItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Spot Shot Power", nullptr), 
        QCoreApplication::translate("Config", "Spot shot power", nullptr));

    UserRegister::fillingSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Filling Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "Filling speed", nullptr));

    UserRegister::fillingStartSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Filling Start Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "Filling start speed", nullptr));

    UserRegister::fillingAccelerationItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Filling Acceleration(mm/s<sup>2</sup>)", nullptr), 
        QCoreApplication::translate("Config", "Filling acceleration", nullptr));

    UserRegister::maxFillingPowerItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Max Filling Power(%)", nullptr), 
        QCoreApplication::translate("Config", "Max filling power", nullptr));

    UserRegister::minFillingPowerItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Min Filling Power(%)", nullptr), 
        QCoreApplication::translate("Config", "Min filling power", nullptr));

    UserRegister::fillingAccRatioItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Filling Acceleration Ratio", nullptr), 
        QCoreApplication::translate("Config", "Filling acceleration ratio", nullptr));

    SystemRegister::headItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Head Data", nullptr), 
        QCoreApplication::translate("Config", "Read-only data used to test the quality of data transmission", nullptr));

    SystemRegister::passwordItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Password", nullptr), 
        QCoreApplication::translate("Config", "Manufacture password to modify system registers", nullptr));

    SystemRegister::storedPasswordItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Stored Password", nullptr), 
        QCoreApplication::translate("Config", "Stored manufacture password", nullptr));

    SystemRegister::hardwareID1Item()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Hardware ID1", nullptr), 
        QCoreApplication::translate("Config", "Hardware ID1", nullptr));

    SystemRegister::hardwareID2Item()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Hardware ID2", nullptr), 
        QCoreApplication::translate("Config", "Hardware ID2", nullptr));

    SystemRegister::hardwareID3Item()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Hardware ID3", nullptr), 
        QCoreApplication::translate("Config", "Hardware ID3", nullptr));

    SystemRegister::cdKey1Item()->setTitleAndDesc(
        QCoreApplication::translate("Config", "CDKey1", nullptr), 
        QCoreApplication::translate("Config", "CDKey1", nullptr));

    SystemRegister::cdKey2Item()->setTitleAndDesc(
        QCoreApplication::translate("Config", "CDKey2", nullptr), 
        QCoreApplication::translate("Config", "CDKey2", nullptr));

    SystemRegister::cdKey3Item()->setTitleAndDesc(
        QCoreApplication::translate("Config", "CDKey3", nullptr), 
        QCoreApplication::translate("Config", "CDKey3", nullptr));

    SystemRegister::sysRunTimeItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "System Run Time", nullptr), 
        QCoreApplication::translate("Config", "System run time", nullptr));

    SystemRegister::laserRunTimeItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Laser Run Time", nullptr), 
        QCoreApplication::translate("Config", "Laser run time", nullptr));

    SystemRegister::sysRunNumItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "System Run Times", nullptr), 
        QCoreApplication::translate("Config", "System run times", nullptr));

    SystemRegister::xMaxLengthItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "X Max Length(mm)", nullptr), 
        QCoreApplication::translate("Config", "X max length", nullptr));

    SystemRegister::xDirPhaseItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "X Dir Phase", nullptr), 
        QCoreApplication::translate("Config", "X dir phase", nullptr));

    SystemRegister::xLimitPhaseItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "X Limit Phase", nullptr), 
        QCoreApplication::translate("Config", "X limit phase", nullptr));

    SystemRegister::xZeroDevItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "X Zero Dev(mm)", nullptr), 
        QCoreApplication::translate("Config", "X Zero Dev", nullptr));

    SystemRegister::xStepLengthItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "X Step Length(mm)", nullptr), 
        QCoreApplication::translate("Config", "X Zero Dev", nullptr));

    SystemRegister::xLimitNumItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "X Limit Num", nullptr), 
        QCoreApplication::translate("Config", "X limit num", nullptr));

    SystemRegister::xResetEnabledItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "X Reset Enabled", nullptr), 
        QCoreApplication::translate("Config", "Enable x reset", nullptr));

    SystemRegister::xMotorNumItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "X Motor Num", nullptr), 
        QCoreApplication::translate("Config", "X motor num", nullptr));

    SystemRegister::xMotorCurrentItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "X Motor current(%)", nullptr), 
        QCoreApplication::translate("Config", "X motor current", nullptr));

    SystemRegister::xStartSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "X Start Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "X Start Speed", nullptr));

    SystemRegister::xMaxSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "X Max Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "X Max Speed", nullptr));

    SystemRegister::xMaxAccelerationItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "X Max Acceleration(mm/s<sup>2</sup>)", nullptr), 
        QCoreApplication::translate("Config", "X max acceleration", nullptr));

    SystemRegister::xUrgentAccelerationItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "X Urgent Acceleration(mm/s<sup>2</sup>)", nullptr), 
        QCoreApplication::translate("Config", "X urgent acceleration", nullptr));

    SystemRegister::yMaxLengthItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Y Max Length(mm)", nullptr), 
        QCoreApplication::translate("Config", "Y max length", nullptr));

    SystemRegister::yDirPhaseItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Y Dir Phase", nullptr), 
        QCoreApplication::translate("Config", "Y dir phase", nullptr));

    SystemRegister::yLimitPhaseItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Y Limit Phase", nullptr), 
        QCoreApplication::translate("Config", "Y limit phase", nullptr));

    SystemRegister::yZeroDevItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Y Zero Dev(mm)", nullptr), 
        QCoreApplication::translate("Config", "Y Zero Dev", nullptr));

    SystemRegister::yStepLengthItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Y Step Length(mm)", nullptr), 
        QCoreApplication::translate("Config", "Y Zero Dev", nullptr));

    SystemRegister::yLimitNumItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Y Limit Num", nullptr), 
        QCoreApplication::translate("Config", "Y limit num", nullptr));

    SystemRegister::yResetEnabledItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Y Reset Enabled", nullptr), 
        QCoreApplication::translate("Config", "Enable y reset", nullptr));

    SystemRegister::yMotorNumItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Y Motor Num", nullptr), 
        QCoreApplication::translate("Config", "Y motor num", nullptr));

    SystemRegister::yMotorCurrentItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Y Motor current(%)", nullptr), 
        QCoreApplication::translate("Config", "Y motor current", nullptr));

    SystemRegister::yStartSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Y Start Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "Y Start Speed", nullptr));

    SystemRegister::yMaxSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Y Max Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "Y Max Speed", nullptr));

    SystemRegister::yMaxAccelerationItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Y Max Acceleration(mm/s<sup>2</sup>)", nullptr), 
        QCoreApplication::translate("Config", "Y max acceleration", nullptr));

    SystemRegister::yUrgentAccelerationItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Y Urgent Acceleration(mm/s<sup>2</sup>)", nullptr), 
        QCoreApplication::translate("Config", "Y urgent acceleration", nullptr));

    SystemRegister::zMaxLengthItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Z Max Length(mm)", nullptr), 
        QCoreApplication::translate("Config", "Y max length", nullptr));

    SystemRegister::zDirPhaseItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Z Dir Phase", nullptr), 
        QCoreApplication::translate("Config", "Z dir phase", nullptr));

    SystemRegister::zLimitPhaseItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Z Limit Phase", nullptr), 
        QCoreApplication::translate("Config", "Z limit phase", nullptr));

    SystemRegister::zZeroDevItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Z Zero Dev(mm)", nullptr), 
        QCoreApplication::translate("Config", "Z Zero Dev", nullptr));

    SystemRegister::zStepLengthItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Z Step Length(mm)", nullptr), 
        QCoreApplication::translate("Config", "Z Zero Dev", nullptr));

    SystemRegister::zLimitNumItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Z Limit Num", nullptr), 
        QCoreApplication::translate("Config", "Z limit num", nullptr));

    SystemRegister::zResetEnabledItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Z Reset Enabled", nullptr), 
        QCoreApplication::translate("Config", "Enable z reset", nullptr));

    SystemRegister::zMotorNumItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Z Motor Num", nullptr), 
        QCoreApplication::translate("Config", "Z motor num", nullptr));

    SystemRegister::zMotorCurrentItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Z Motor current(%)", nullptr), 
        QCoreApplication::translate("Config", "Z motor current", nullptr));

    SystemRegister::zStartSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Z Start Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "Z Start Speed", nullptr));

    SystemRegister::zMaxSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Z Max Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "Z Max Speed", nullptr));

    SystemRegister::zMaxAccelerationItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Z Max Acceleration(mm/s<sup>2</sup>)", nullptr), 
        QCoreApplication::translate("Config", "Z max acceleration", nullptr));

    SystemRegister::zUrgentAccelerationItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Z Urgent Acceleration(mm/s<sup>2</sup>)", nullptr), 
        QCoreApplication::translate("Config", "Z urgent acceleration", nullptr));

    SystemRegister::laserMaxPowerItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Laser Max Power(%)", nullptr),
        QCoreApplication::translate("Config", "Laser Max Power", nullptr));

    SystemRegister::laserMinPowerItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Laser Min Power(%)", nullptr),
        QCoreApplication::translate("Config", "Laser Min Power", nullptr));

    SystemRegister::laserPowerFreqItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Laser Power Frequence", nullptr),
        QCoreApplication::translate("Config", "Laser Power Frequence", nullptr));

    SystemRegister::xPhaseEnabledItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "X Phase Enabled", nullptr),
        QCoreApplication::translate("Config", "Enabled X phase", nullptr));

    SystemRegister::yPhaseEnabledItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Y Phase Enabled", nullptr),
        QCoreApplication::translate("Config", "Enabled Y phase", nullptr));

    SystemRegister::zPhaseEnabledItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Z Phase Enabled", nullptr),
        QCoreApplication::translate("Config", "Enabled Z phase", nullptr));

    SystemRegister::deviceOriginItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Device Origin", nullptr),
        QCoreApplication::translate("Config", "Device Origin", nullptr));

    SystemRegister::zResetSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Z Reset Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "Z Reset Speed", nullptr));

    Debug::showPrimitiveNameItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Show Primitive Name", nullptr),
        QCoreApplication::translate("Config", "Show Primitive Name", nullptr));

    Debug::showPrimitiveFirstPointItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Show Primitive First Point", nullptr),
        QCoreApplication::translate("Config", "Show Primitive First Point", nullptr));

    Debug::generatePathImageItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Generate Path Image", nullptr),
        QCoreApplication::translate("Config", "Enabled generate path image", nullptr));

    Debug::generateMachiningImageItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Generate Machining Image", nullptr),
        QCoreApplication::translate("Config", "Enabled generate machining image", nullptr));

    Debug::enableOptimizeInteractionItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Enable Optimize Interaction", nullptr),
        QCoreApplication::translate("Config", "Enable Optimize Interaction", nullptr));

    groupsMap["general"]->updateTitleAndDesc(
        QCoreApplication::translate("Config", "General", nullptr),
        QCoreApplication::translate("Config", "General", nullptr));

    groupsMap["layers"]->updateTitleAndDesc(
        QCoreApplication::translate("Config", "Layers", nullptr),
        QCoreApplication::translate("Config", "Layers", nullptr));

    groupsMap["ui"]->updateTitleAndDesc(
        QCoreApplication::translate("Config", "UI", nullptr),
        QCoreApplication::translate("Config", "UI", nullptr));

    groupsMap["cuttingLayer"]->updateTitleAndDesc(
        QCoreApplication::translate("Config", "Cutting Layer", nullptr),
        QCoreApplication::translate("Config", "Cutting Layer", nullptr));

    groupsMap["engravingLayer"]->updateTitleAndDesc(
        QCoreApplication::translate("Config", "Engraving Layer", nullptr),
        QCoreApplication::translate("Config", "Engraving Layer", nullptr));

    groupsMap["fillingLayer"]->updateTitleAndDesc(
        QCoreApplication::translate("Config", "Filling Layer", nullptr),
        QCoreApplication::translate("Config", "Filling Layer", nullptr));

    groupsMap["pathOptimization"]->updateTitleAndDesc(
        QCoreApplication::translate("Config", "Path Optimization", nullptr),
        QCoreApplication::translate("Config", "Path Optimization", nullptr));

    groupsMap["export"]->updateTitleAndDesc(
        QCoreApplication::translate("Config", "Export", nullptr),
        QCoreApplication::translate("Config", "Export", nullptr));

    groupsMap["device"]->updateTitleAndDesc(
        QCoreApplication::translate("Config", "Device", nullptr),
        QCoreApplication::translate("Config", "Device", nullptr));

    groupsMap["debug"]->updateTitleAndDesc(
        QCoreApplication::translate("Config", "Debug", nullptr),
        QCoreApplication::translate("Config", "Debug", nullptr));

    groupsMap["userRegister"]->updateTitleAndDesc(
        QCoreApplication::translate("Config", "User", nullptr),
        QCoreApplication::translate("Config", "User", nullptr));

    groupsMap["systemRegister"]->updateTitleAndDesc(
        QCoreApplication::translate("Config", "System", nullptr),
        QCoreApplication::translate("Config", "System", nullptr));
}

void Config::destroy()
{
    save(true, true);
    qDeleteAll(groups);
    groups.clear();
    groupsMap.clear();
}
