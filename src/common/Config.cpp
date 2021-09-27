#include "Config.h"

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
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

            //comboBox->addItem(QCoreApplication::translate("Config", ("Off"), nullptr), 0);
            //comboBox->addItem(QCoreApplication::translate("Config", ("Low Contrast"), nullptr), 1);
            //comboBox->addItem(QCoreApplication::translate("Config", ("Medium Contrast"), nullptr), 2);
            //comboBox->addItem(QCoreApplication::translate("Config", ("High Contrast"), nullptr), 3);

            comboBox->addItem(item->extraProperty("Off").toString(), 0);
            comboBox->addItem(item->extraProperty("Low Contrast").toString(), 1);
            comboBox->addItem(item->extraProperty("Medium Contrast").toString(), 2);
            comboBox->addItem(item->extraProperty("High Contrast").toString(), 3);

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
		10
	);
    visualGridSpacing->setInputWidgetProperty("textTemplate", "%1mm");
	visualGridSpacing->setInputWidgetProperty("minimum", 0);
	visualGridSpacing->setInputWidgetProperty("maximum", 10);

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

    /*ConfigItem* autoRepeatInterval = group->addConfigItem(
        "autoRepeatInterval",
        tr("Auto repeat interval"),
        tr("Auto repeat interval when pressing a button"),
        200
    );
    autoRepeatInterval->setInputWidgetProperty("textTemplate", "%1ms");
    autoRepeatInterval->setInputWidgetProperty("minimum", 0);
    autoRepeatInterval->setInputWidgetProperty("maximum", 2000);*/
}

void Config::loadCuttingLayerItems()
{
    ConfigItemGroup* group = new Config::CuttingLayer;
    Config::CuttingLayer::group = group;

    ConfigItem* minSpeed = group->addConfigItem(
        "minSpeed",
        15
    );
    minSpeed->setInputWidgetProperty("minimum", 1);
    minSpeed->setInputWidgetProperty("maximum", 1000);
    minSpeed->setInputWidgetProperty("textTemplate", "%1mm/s");
    minSpeed->setInputWidgetProperty("maximumLineEditWidth", 60);

    ConfigItem* runSpeed = group->addConfigItem(
        "runSpeed",
        60
    );
    runSpeed->setInputWidgetProperty("minimum", 1);
    runSpeed->setInputWidgetProperty("maximum", 1000);
    runSpeed->setInputWidgetProperty("textTemplate", "%1mm/s");
    runSpeed->setInputWidgetProperty("maximumLineEditWidth", 60);

    ConfigItem* laserPower = group->addConfigItem(
        "laserPower",
        8,
        DT_REAL
    );
    laserPower->setInputWidgetProperty("minimum", 0);
    laserPower->setInputWidgetProperty("maximum", 100);
    laserPower->setInputWidgetProperty("textTemplate", "%1%");

    ConfigItem* minPower = group->addConfigItem(
        "minPower",
        70,
        DT_REAL
    );
    minPower->setInputWidgetProperty("minimum", 0);
    minPower->setInputWidgetProperty("maximum", 100);
    minPower->setInputWidgetProperty("textTemplate", "%1%");

    ConfigItem* maxPower = group->addConfigItem(
        "maxPower",
        100,
        DT_REAL
    );
    maxPower->setInputWidgetProperty("minimum", 0);
    maxPower->setInputWidgetProperty("maximum", 100);
    maxPower->setInputWidgetProperty("textTemplate", "%1%");
}

void Config::loadEngravingLayerItems()
{
    ConfigItemGroup* group = new Config::EngravingLayer;
    Config::EngravingLayer::group = group;

    ConfigItem* minSpeed = group->addConfigItem(
        "minSpeed",
        15
    );
    minSpeed->setInputWidgetProperty("minimum", 1);
    minSpeed->setInputWidgetProperty("maximum", 1000);
    minSpeed->setInputWidgetProperty("textTemplate", "%1mm/s");
    minSpeed->setInputWidgetProperty("maximumLineEditWidth", 60);

    ConfigItem* runSpeed = group->addConfigItem(
        "runSpeed",
        60
    );
    runSpeed->setInputWidgetProperty("minimum", 1);
    runSpeed->setInputWidgetProperty("maximum", 1000);
    runSpeed->setInputWidgetProperty("textTemplate", "%1mm/s");
    runSpeed->setInputWidgetProperty("maximumLineEditWidth", 60);

    ConfigItem* laserPower = group->addConfigItem(
        "laserPower",
        8,
        DT_REAL
    );
    laserPower->setInputWidgetType(IWT_FloatEditSlider);
    laserPower->setInputWidgetProperty("minimum", 0);
    laserPower->setInputWidgetProperty("maximum", 100);
    laserPower->setInputWidgetProperty("textTemplate", "%1%");

    ConfigItem* minPower= group->addConfigItem(
        "minPower",
        70,
        DT_REAL
    );
    minPower->setInputWidgetType(IWT_FloatEditSlider);
    minPower->setInputWidgetProperty("minimum", 0);
    minPower->setInputWidgetProperty("maximum", 100);
    minPower->setInputWidgetProperty("textTemplate", "%1%");

    ConfigItem* maxPower= group->addConfigItem(
        "maxPower",
        100,
        DT_REAL
    );
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
}

void Config::loadFillingLayerItems()
{
    ConfigItemGroup* group = new Config::FillingLayer;
    Config::FillingLayer::group = group;

    ConfigItem* minSpeed = group->addConfigItem(
        "minSpeed",
        15
    );
    minSpeed->setInputWidgetProperty("minimum", 1);
    minSpeed->setInputWidgetProperty("maximum", 1000);
    minSpeed->setInputWidgetProperty("textTemplate", "%1mm/s");
    minSpeed->setInputWidgetProperty("maximumLineEditWidth", 60);

    ConfigItem* runSpeed = group->addConfigItem(
        "runSpeed",
        30
    );
    runSpeed->setInputWidgetProperty("minimum", 1);
    runSpeed->setInputWidgetProperty("maximum", 1000);
    runSpeed->setInputWidgetProperty("textTemplate", "%1mm/s");
    runSpeed->setInputWidgetProperty("maximumLineEditWidth", 60);

    ConfigItem* laserPower = group->addConfigItem(
        "laserPower",
        8,
        DT_REAL
    );
    laserPower->setInputWidgetType(IWT_FloatEditSlider);
    laserPower->setInputWidgetProperty("minimum", 0);
    laserPower->setInputWidgetProperty("maximum", 100);
    laserPower->setInputWidgetProperty("textTemplate", "%1%");

    ConfigItem* minPower= group->addConfigItem(
        "minPower",
        4,
        DT_REAL
    );
    minPower->setInputWidgetType(IWT_FloatEditSlider);
    minPower->setInputWidgetProperty("minimum", 0);
    minPower->setInputWidgetProperty("maximum", 100);
    minPower->setInputWidgetProperty("textTemplate", "%1%");

    ConfigItem* maxPower= group->addConfigItem(
        "maxPower",
        12,
        DT_REAL
    );
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

            comboBox->addItem(tr("Horizontal"), 1);
            comboBox->addItem(tr("Vertical"), 2);

            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );

    ConfigItem* groupingGridInterval = group->addConfigItem(
        "groupingGridInterval",
        30,
        DT_REAL
    );
    groupingGridInterval->setInputWidgetType(IWT_FloatEditSlider);
    groupingGridInterval->setInputWidgetProperty("minimum", 1.0);
    groupingGridInterval->setInputWidgetProperty("maximum", 1000.0);

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
    enableSmallDiagonal->setStoreStrategy(SS_DIRECTLY);

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
        [=](const QVariant& value, ModifiedBy modifiedBy) {
            bool enabled = value.toBool();
            smallDiagonalLimitation->setEnabled(enabled);
        }
    );

    ConfigItem* enableRelativeCoordinates = group->addConfigItem(
        "enableRelativeCoordinates",
        false,
        DT_BOOL
    );
    enableRelativeCoordinates->setStoreStrategy(SS_DIRECTLY);

    ConfigItem* imageUseGaussian = group->addConfigItem(
        "imageUseGaussian",
        false,
        DT_BOOL
    );
    imageUseGaussian->setVisible(false);
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
    startFrom->setStoreStrategy(SS_DIRECTLY);
    startFrom->setWidgetInitializeHook(
        [](QWidget* widget, ConfigItem* item, InputWidgetWrapper* wrapper)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (!comboBox)
                return;

            //comboBox->addItem(tr("Current Position"), SFT_CurrentPosition);
            //comboBox->addItem(tr("User Origin"), SFT_UserOrigin);
            //comboBox->addItem(tr("Absolute Coords"), SFT_AbsoluteCoords);
            comboBox->addItem(item->extraProperty("Current Position").toString(), SFT_CurrentPosition);
            comboBox->addItem(item->extraProperty("User Origin").toString(), SFT_UserOrigin);
            comboBox->addItem(item->extraProperty("Absolute Coords").toString(), SFT_AbsoluteCoords);

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
        true,
        DT_BOOL
    );
    xEnabled->setStoreStrategy(SS_DIRECTLY);

    ConfigItem* yEnabled = group->addConfigItem(
        "yEnabled",
        true,
        DT_BOOL
    );
    yEnabled->setStoreStrategy(SS_DIRECTLY);

    ConfigItem* zEnabled = group->addConfigItem(
        "zEnabled",
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
        0x12345678,
        DT_INT
    );
    head->setReadOnly();
    head->setInputWidgetType(IWT_LineEdit);
    head->setInputWidgetProperty("readOnly", true);

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

    ConfigItem* cuttingMoveSpeed = group->addConfigItem(
        "cuttingMoveSpeed",
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
        2
    );
    cuttingMoveSpeedFactor->setInputWidgetProperty("maximumLineEditWidth", 75);
    cuttingMoveSpeedFactor->setInputWidgetProperty("minimum", 1);
    cuttingMoveSpeedFactor->setInputWidgetProperty("maximum", 100);

    ConfigItem* cuttingWorkSpeedFactor = group->addConfigItem(
        "cuttingWorkSpeedFactor",
        2
    );
    cuttingWorkSpeedFactor->setInputWidgetProperty("maximumLineEditWidth", 75);
    cuttingWorkSpeedFactor->setInputWidgetProperty("minimum", 1);
    cuttingWorkSpeedFactor->setInputWidgetProperty("maximum", 100);

    ConfigItem* cuttingSpotSize = group->addConfigItem(
        "cuttingSpotSize",
        1000,
        DT_INT
    );
    cuttingSpotSize->setInputWidgetProperty("maximumLineEditWidth", 75);
    cuttingSpotSize->setInputWidgetProperty("minimum", 1);
    cuttingSpotSize->setInputWidgetProperty("maximum", 1000);

    ConfigItem* scanXStartSpeed = group->addConfigItem(
        "scanXStartSpeed",
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
    resetSpeed->setInputWidgetProperty("textTemplate", "%1mm/s");
    resetSpeed->setInputWidgetProperty("step", 0.001);
    resetSpeed->setInputWidgetProperty("page", 10);
    resetSpeed->setInputWidgetProperty("minimum", 0.001);
    resetSpeed->setInputWidgetProperty("maximum", 10000);

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
    defaultRunSpeed->setInputWidgetProperty("textTemplate", "%1mm/s");
    defaultRunSpeed->setInputWidgetProperty("step", 0.001);
    defaultRunSpeed->setInputWidgetProperty("page", 10);
    defaultRunSpeed->setInputWidgetProperty("minimum", 1);
    defaultRunSpeed->setInputWidgetProperty("maximum", 10000);

    ConfigItem* defaultMaxCuttingPower = group->addConfigItem(
        "defaultMaxCuttingPower",
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
    defaultScanSpeed->setInputWidgetProperty("textTemplate", "%1%");
    defaultScanSpeed->setInputWidgetProperty("step", 0.001);
    defaultScanSpeed->setInputWidgetProperty("page", 10);
    defaultScanSpeed->setInputWidgetProperty("minimum", 0.001);
    defaultScanSpeed->setInputWidgetProperty("maximum", 10000);

    ConfigItem* maxScanGrayRatio = group->addConfigItem(
        "maxScanGrayRatio",
        800,
        DT_INT
    );
    maxScanGrayRatio->setInputWidgetProperty("maximumLineEditWidth", 75);
    maxScanGrayRatio->setInputWidgetProperty("minimum", 1);
    maxScanGrayRatio->setInputWidgetProperty("maximum", 2000);

    ConfigItem* minScanGrayRatio = group->addConfigItem(
        "minScanGrayRatio",
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
        0x12345678,
        DT_INT
    );
    head->setReadOnly();
    head->setInputWidgetType(IWT_LineEdit);
    head->setInputWidgetProperty("readOnly", true);

    ConfigItem* password = group->addConfigItem(
        "password",
        "",
        DT_STRING
    );
    password->setWriteOnly();
    password->setInputWidgetType(IWT_LineEdit);
    password->setVisible(false);

    ConfigItem* storedPassword = group->addConfigItem(
        "storedPassword",
        "",
        DT_STRING
    );
    storedPassword->setWriteOnly();
    storedPassword->setInputWidgetType(IWT_LineEdit);
    storedPassword->setVisible(false);

    ConfigItem* hardwareID1 = group->addConfigItem(
        "hardwareID1",
        "",
        DT_STRING
    );
    hardwareID1->setReadOnly();
    hardwareID1->setInputWidgetType(IWT_LineEdit);
    hardwareID1->setInputWidgetProperty("readOnly", true);

    ConfigItem* hardwareID2 = group->addConfigItem(
        "hardwareID2",
        "",
        DT_STRING
    );
    hardwareID2->setReadOnly();
    hardwareID2->setInputWidgetType(IWT_LineEdit);
    hardwareID2->setInputWidgetProperty("readOnly", true);

    ConfigItem* hardwareID3 = group->addConfigItem(
        "hardwareID3",
        "",
        DT_STRING
    );
    hardwareID3->setReadOnly();
    hardwareID3->setInputWidgetType(IWT_LineEdit);
    hardwareID3->setInputWidgetProperty("readOnly", true);

    ConfigItem* cdKey1 = group->addConfigItem(
        "cdKey1",
        "",
        DT_STRING
    );
    cdKey1->setInputWidgetType(IWT_LineEdit);

    ConfigItem* cdKey2 = group->addConfigItem(
        "cdKey2",
        "",
        DT_STRING
    );
    cdKey2->setInputWidgetType(IWT_LineEdit);

    ConfigItem* cdKey3 = group->addConfigItem(
        "cdKey3",
        "",
        DT_STRING
    );
    cdKey3->setInputWidgetType(IWT_LineEdit);

    ConfigItem* sysRunTime = group->addConfigItem(
        "sysRunTime",
        0,
        DT_INT
    );
    sysRunTime->setReadOnly(true);
    sysRunTime->setInputWidgetType(IWT_LineEdit);

    ConfigItem* laserRunTime = group->addConfigItem(
        "laserRunTime",
        0,
        DT_INT
    );
    laserRunTime->setReadOnly(true);
    laserRunTime->setInputWidgetType(IWT_LineEdit);

    ConfigItem* sysRunNum = group->addConfigItem(
        "sysRunNum",
        0,
        DT_INT
    );
    sysRunNum->setReadOnly(true);
    sysRunNum->setInputWidgetType(IWT_LineEdit);

    ConfigItem* xMaxLength = group->addConfigItem(
        "xMaxLength",
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
    xMaxLength->setInputWidgetProperty("textTemplate", "%1mm");
    xMaxLength->setInputWidgetProperty("step", 0.001);
    xMaxLength->setInputWidgetProperty("page", 10);
    xMaxLength->setInputWidgetProperty("minimum", 1);
    xMaxLength->setInputWidgetProperty("maximum", 5000);
    xMaxLength->setInputWidgetProperty("decimals", 3);

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

            comboBox->addItem(tr("0"), 0);
            comboBox->addItem(tr("1"), 1);

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

            comboBox->addItem(tr("0"), 0);
            comboBox->addItem(tr("1"), 1);

            int index = widgetUtils::findComboBoxIndexByValue(comboBox, item->value());
            comboBox->setCurrentIndex(index < 0 ? widgetUtils::findComboBoxIndexByValue(comboBox, item->defaultValue()) : index);
        }
    );

    ConfigItem* xZeroDev = group->addConfigItem(
        "xZeroDev",
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
    xZeroDev->setInputWidgetProperty("textTemplate", "%1mm");
    xZeroDev->setInputWidgetProperty("step", 0.001);
    xZeroDev->setInputWidgetProperty("page", 1);
    xZeroDev->setInputWidgetProperty("minimum", 0);
    xZeroDev->setInputWidgetProperty("maximum", 10);
    xZeroDev->setInputWidgetProperty("decimals", 3);

    ConfigItem* xStepLength = group->addConfigItem(
        "xStepLength",
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
        true,
        DT_BOOL
    );

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
        true,
        DT_BOOL
    );

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
        true,
        DT_BOOL
    );

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
        4000
    );
    laserPowerFreq->setInputWidgetProperty("maximumLineEditWidth", 75);
    laserPowerFreq->setInputWidgetProperty("page", 1000);
    laserPowerFreq->setInputWidgetProperty("minimum", 1);
    laserPowerFreq->setInputWidgetProperty("maximum", 10000);
    laserPowerFreq->setStoreStrategy(SS_DIRECTLY);

    ConfigItem* xPhaseEnabled = group->addConfigItem(
        "xPhaseEnabled",
        true,
        DT_BOOL
    );

    ConfigItem* yPhaseEnabled = group->addConfigItem(
        "yPhaseEnabled",
        true,
        DT_BOOL
    );

    ConfigItem* zPhaseEnabled = group->addConfigItem(
        "zPhaseEnabled",
        true,
        DT_BOOL
    );

    ConfigItem* deviceOrigin = group->addConfigItem(
        "deviceOrigin",
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
        QCoreApplication::translate("Config", "Language", nullptr), 
        QCoreApplication::translate("Config", "Language for both UI and Business.", nullptr));

    General::machiningUnitItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Machining Unit", nullptr), 
        QCoreApplication::translate("Config", "Unit for machining", nullptr));

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
    Ui::gridContrastItem()->setExtraProperty("Off", QCoreApplication::translate("Config", "Off", nullptr));
    Ui::gridContrastItem()->setExtraProperty("Low Contrast", QCoreApplication::translate("Config", "Low Contrast", nullptr));
    Ui::gridContrastItem()->setExtraProperty("Medium Contrast", QCoreApplication::translate("Config", "Medium Contrast", nullptr));
    Ui::gridContrastItem()->setExtraProperty("High Contrast", QCoreApplication::translate("Config", "High Contrast", nullptr));

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

    Ui::splitterHandleWidthItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Splitter Handle Width(px)", nullptr), 
        QCoreApplication::translate("Config", "Width of splitter handle in px", nullptr));

    Ui::autoRepeatDelayItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Auto repeat delay(ms)", nullptr), 
        QCoreApplication::translate("Config", "The delay duration of auto repeat button", nullptr));

    CuttingLayer::minSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Min Speed(mm)", nullptr), 
        QCoreApplication::translate("Config", "Min speed for cutting layers", nullptr));

    CuttingLayer::runSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Run Speed(mm)", nullptr), 
        QCoreApplication::translate("Config", "Run speed for cutting layers", nullptr));

    CuttingLayer::laserPowerItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Laser Power(%)", nullptr), 
        QCoreApplication::translate("Config", "The power percentage for cutting layers", nullptr));

    CuttingLayer::minPowerItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Min Laser Power(%)", nullptr), 
        QCoreApplication::translate("Config", "The min power percentage for cutting layers", nullptr));

    CuttingLayer::maxPowerItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Max Laser Power(%)", nullptr), 
        QCoreApplication::translate("Config", "The max power percentage for cutting layers", nullptr));

    EngravingLayer::minSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Min Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "The min speed for engraving layers", nullptr));

    EngravingLayer::runSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Run Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "The run speed for engraving layers", nullptr));

    EngravingLayer::laserPowerItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Laser Power(%)", nullptr), 
        QCoreApplication::translate("Config", "The laser power for engraving layers", nullptr));

    EngravingLayer::minPowerItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Min Power(%)", nullptr), 
        QCoreApplication::translate("Config", "The min power percentage for engraving layers", nullptr));

    EngravingLayer::maxPowerItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Max Power(%)", nullptr), 
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

    FillingLayer::minSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Min Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "The min speed for filling layers", nullptr));

    FillingLayer::runSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Run Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "The run speed for filling layers", nullptr));

    FillingLayer::laserPowerItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Laser Power(%)", nullptr), 
        QCoreApplication::translate("Config", "The laser power for filling layers", nullptr));

    FillingLayer::minPowerItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Min Power(%)", nullptr), 
        QCoreApplication::translate("Config", "The min power percentage for filling layers", nullptr));

    FillingLayer::maxPowerItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Max Power(%)", nullptr), 
        QCoreApplication::translate("Config", "The max power percentage for filling layers", nullptr));

    FillingLayer::rowIntervalItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Row Interval(μm)", nullptr), 
        QCoreApplication::translate("Config", "The row interval between lines of bitmap for filling layers", nullptr));

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

    Export::enableRelativeCoordinatesItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Enable Relative Coordinates", nullptr), 
        QCoreApplication::translate("Config", "Enable relative coordinates for exporting machining points", nullptr));

    Export::imageUseGaussianItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Image Use Gaussian", nullptr), 
        QCoreApplication::translate("Config", "Use gaussian when generating images", nullptr));

    Device::autoConnectFirstCOMItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Auto Connect First COM", nullptr), 
        QCoreApplication::translate("Config", "Auto connect to first com port when found multiple laser devices", nullptr));

    Device::startFromItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "Start from", nullptr), 
        QCoreApplication::translate("Config", "Choose the start point type of machining", nullptr));
    Device::startFromItem()->setExtraProperty("Current Position", QCoreApplication::translate("Config", "Current Position", nullptr));
    Device::startFromItem()->setExtraProperty("User Origin", QCoreApplication::translate("Config", "User Origin", nullptr));
    Device::startFromItem()->setExtraProperty("Absolute Coords", QCoreApplication::translate("Config", "Absolute Coords", nullptr));

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

    UserRegister::headItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[00] Head Data", nullptr), 
        QCoreApplication::translate("Config", "Read-only data used to test the quality of data transmission", nullptr));

    UserRegister::accModeItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[01] Acceleration Mode", nullptr), 
        QCoreApplication::translate("Config", "Acceleration mode", nullptr));

    UserRegister::cuttingMoveSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[02] Cutting Move Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "Acceleration mode", nullptr));

    UserRegister::cuttingMoveAccItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[03] Cutting Move Acceleration(mm/s<sub>2</sub>)", nullptr), 
        QCoreApplication::translate("Config", "Move acceleration for cutting movement", nullptr));

    UserRegister::cuttingTurnSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[04] Cutting Turn Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "Turn speed for cutting movement", nullptr));

    UserRegister::cuttingTurnAccItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[05] Cutting Turn Acceleration(mm/s<sub>2</sub>)", nullptr), 
        QCoreApplication::translate("Config", "Turn acceleration for cutting movement", nullptr));

    UserRegister::cuttingWorkAccItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[06] Cutting Work Acceleration(mm/s<sub>2</sub>)", nullptr), 
        QCoreApplication::translate("Config", "Work acceleration for cutting movement", nullptr));

    UserRegister::cuttingMoveSpeedFactorItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[07] Move Speed Factor", nullptr), 
        QCoreApplication::translate("Config", "Move speed factor for cutting movement", nullptr));

    UserRegister::cuttingWorkSpeedFactorItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[08] Work Speed Factor", nullptr), 
        QCoreApplication::translate("Config", "Work speed factor for cutting movement", nullptr));

    UserRegister::cuttingSpotSizeItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[09] Cutting Spot Size", nullptr), 
        QCoreApplication::translate("Config", "Spot size for cutting movement", nullptr));

    UserRegister::scanXStartSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[10] Scan X Start Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "Scan x start speed", nullptr));

    UserRegister::scanYStartSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[11] Scan Y Start Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "Scan y start speed", nullptr));

    UserRegister::scanXAccItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[12] Scan X Acceleration(mm/s<sub>2</sub>)", nullptr), 
        QCoreApplication::translate("Config", "Scan x acceleration", nullptr));

    UserRegister::scanYAccItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[13] Scan Y Acceleration(mm/s<sub>2</sub>)", nullptr), 
        QCoreApplication::translate("Config", "Scan y acceleration", nullptr));

    UserRegister::scanRowSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[14] Scan Row Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "Scan row speed", nullptr));

    UserRegister::scanRowIntervalItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[15] Scan Row Interval(mm)", nullptr), 
        QCoreApplication::translate("Config", "Scan row interval", nullptr));

    UserRegister::scanReturnErrorItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[16] Scan Return Error(mm)", nullptr), 
        QCoreApplication::translate("Config", "Scan return error", nullptr));

    UserRegister::scanLaserPowerItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[17] Scan Laser Power(%)", nullptr), 
        QCoreApplication::translate("Config", "The percentage of scan laser power", nullptr));

    UserRegister::scanXResetEnabledItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[18] Scan X Reset Enabled", nullptr), 
        QCoreApplication::translate("Config", "Enabled Scan X reset", nullptr));

    UserRegister::scanYResetEnabledItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[19] Scan Y Reset Enabled", nullptr), 
        QCoreApplication::translate("Config", "Enabled Scan Y reset", nullptr));

    UserRegister::scanZResetEnabledItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[20] Scan Z Reset Enabled", nullptr), 
        QCoreApplication::translate("Config", "Enabled Scan Z reset", nullptr));

    UserRegister::resetSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[21] Reset Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "Reset speed", nullptr));

    UserRegister::scanReturnPosItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[22] Scan Return Pos", nullptr), 
        QCoreApplication::translate("Config", "Scan return pos", nullptr));

    UserRegister::backlashXIntervalItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[23] Backlash X Interval(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "Backlash X interval", nullptr));

    UserRegister::backlashYIntervalItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[24] Backlash Y Interval(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "Backlash Y interval", nullptr));

    UserRegister::backlashZIntervalItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[25] Backlash Z Interval(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "Backlash Z interval", nullptr));

    UserRegister::defaultRunSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[26] Default Run Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "Default run speed", nullptr));

    UserRegister::defaultMaxCuttingPowerItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[27] Default Max Cutting Power(%)", nullptr), 
        QCoreApplication::translate("Config", "Default max cutting power", nullptr));

    UserRegister::defaultMinCuttingPowerItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[28] Default Min Cutting Power(%)", nullptr), 
        QCoreApplication::translate("Config", "Default min cutting power", nullptr));

    UserRegister::defaultScanSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[29] Default Scan Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "Default scan speed", nullptr));

    UserRegister::maxScanGrayRatioItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[30] Max Scan Gray Ratio", nullptr), 
        QCoreApplication::translate("Config", "Max scan gray ratio", nullptr));

    UserRegister::minScanGrayRatioItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[31] Min Scan Gray Ratio", nullptr), 
        QCoreApplication::translate("Config", "Min scan gray ratio", nullptr));

    SystemRegister::headItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[00] Head Data", nullptr), 
        QCoreApplication::translate("Config", "Read-only data used to test the quality of data transmission", nullptr));

    SystemRegister::passwordItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[01] Password", nullptr), 
        QCoreApplication::translate("Config", "Manufacture password to modify system registers", nullptr));

    SystemRegister::storedPasswordItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[02] Stored Password", nullptr), 
        QCoreApplication::translate("Config", "Stored manufacture password", nullptr));

    SystemRegister::hardwareID1Item()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[03] Hardware ID1", nullptr), 
        QCoreApplication::translate("Config", "Hardware ID1", nullptr));

    SystemRegister::hardwareID2Item()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[04] Hardware ID2", nullptr), 
        QCoreApplication::translate("Config", "Hardware ID2", nullptr));

    SystemRegister::hardwareID3Item()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[05] Hardware ID3", nullptr), 
        QCoreApplication::translate("Config", "Hardware ID3", nullptr));

    SystemRegister::cdKey1Item()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[06] CDKey1", nullptr), 
        QCoreApplication::translate("Config", "CDKey1", nullptr));

    SystemRegister::cdKey2Item()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[07] CDKey2", nullptr), 
        QCoreApplication::translate("Config", "CDKey2", nullptr));

    SystemRegister::cdKey3Item()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[08] CDKey3", nullptr), 
        QCoreApplication::translate("Config", "CDKey3", nullptr));

    SystemRegister::sysRunTimeItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[09] System Run Time", nullptr), 
        QCoreApplication::translate("Config", "System run time", nullptr));

    SystemRegister::laserRunTimeItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[10] Laser Run Time", nullptr), 
        QCoreApplication::translate("Config", "Laser run time", nullptr));

    SystemRegister::sysRunNumItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[11] System Run Times", nullptr), 
        QCoreApplication::translate("Config", "System run times", nullptr));

    SystemRegister::xMaxLengthItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[12] X Max Length(mm)", nullptr), 
        QCoreApplication::translate("Config", "X max length", nullptr));

    SystemRegister::xDirPhaseItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[13] X Dir Phase", nullptr), 
        QCoreApplication::translate("Config", "X dir phase", nullptr));

    SystemRegister::xLimitPhaseItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[14] X Limit Phase", nullptr), 
        QCoreApplication::translate("Config", "X limit phase", nullptr));

    SystemRegister::xZeroDevItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[15] X Zero Dev(mm)", nullptr), 
        QCoreApplication::translate("Config", "X Zero Dev", nullptr));

    SystemRegister::xStepLengthItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[16] X Step Length(mm)", nullptr), 
        QCoreApplication::translate("Config", "X Zero Dev", nullptr));

    SystemRegister::xLimitNumItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[17] X Limit Num", nullptr), 
        QCoreApplication::translate("Config", "X limit num", nullptr));

    SystemRegister::xResetEnabledItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[18] X Reset Enabled", nullptr), 
        QCoreApplication::translate("Config", "Enable x reset", nullptr));

    SystemRegister::xMotorNumItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[19] X Motor Num", nullptr), 
        QCoreApplication::translate("Config", "X motor num", nullptr));

    SystemRegister::xMotorCurrentItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[20] X Motor current(%)", nullptr), 
        QCoreApplication::translate("Config", "X motor current", nullptr));

    SystemRegister::xStartSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[21] X Start Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "X Start Speed", nullptr));

    SystemRegister::xMaxSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[22] X Max Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "X Max Speed", nullptr));

    SystemRegister::xMaxAccelerationItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[23] X Max Acceleration(mm/s<sup>2</sup>)", nullptr), 
        QCoreApplication::translate("Config", "X max acceleration", nullptr));

    SystemRegister::xUrgentAccelerationItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[24] X Urgent Acceleration(mm/s<sup>2</sup>)", nullptr), 
        QCoreApplication::translate("Config", "X urgent acceleration", nullptr));

    SystemRegister::yMaxLengthItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[25] Y Max Length(mm)", nullptr), 
        QCoreApplication::translate("Config", "Y max length", nullptr));

    SystemRegister::yDirPhaseItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[26] Y Dir Phase", nullptr), 
        QCoreApplication::translate("Config", "Y dir phase", nullptr));

    SystemRegister::yLimitPhaseItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[27] Y Limit Phase", nullptr), 
        QCoreApplication::translate("Config", "Y limit phase", nullptr));

    SystemRegister::yZeroDevItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[28] Y Zero Dev(mm)", nullptr), 
        QCoreApplication::translate("Config", "Y Zero Dev", nullptr));

    SystemRegister::yStepLengthItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[29] Y Step Length(mm)", nullptr), 
        QCoreApplication::translate("Config", "Y Zero Dev", nullptr));

    SystemRegister::yLimitNumItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[30] Y Limit Num", nullptr), 
        QCoreApplication::translate("Config", "Y limit num", nullptr));

    SystemRegister::yResetEnabledItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[31] Y Reset Enabled", nullptr), 
        QCoreApplication::translate("Config", "Enable y reset", nullptr));

    SystemRegister::yMotorNumItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[32] Y Motor Num", nullptr), 
        QCoreApplication::translate("Config", "Y motor num", nullptr));

    SystemRegister::yMotorCurrentItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[33] Y Motor current(%)", nullptr), 
        QCoreApplication::translate("Config", "Y motor current", nullptr));

    SystemRegister::yStartSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[34] Y Start Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "Y Start Speed", nullptr));

    SystemRegister::yMaxSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[35] Y Max Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "Y Max Speed", nullptr));

    SystemRegister::yMaxAccelerationItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[36] Y Max Acceleration(mm/s<sup>2</sup>)", nullptr), 
        QCoreApplication::translate("Config", "Y max acceleration", nullptr));

    SystemRegister::yUrgentAccelerationItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[37] Y Urgent Acceleration(mm/s<sup>2</sup>)", nullptr), 
        QCoreApplication::translate("Config", "Y urgent acceleration", nullptr));

    SystemRegister::zMaxLengthItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[38] Z Max Length(mm)", nullptr), 
        QCoreApplication::translate("Config", "Y max length", nullptr));

    SystemRegister::zDirPhaseItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[39] Z Dir Phase", nullptr), 
        QCoreApplication::translate("Config", "Z dir phase", nullptr));

    SystemRegister::zLimitPhaseItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[40] Z Limit Phase", nullptr), 
        QCoreApplication::translate("Config", "Z limit phase", nullptr));

    SystemRegister::zZeroDevItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[41] Z Zero Dev(mm)", nullptr), 
        QCoreApplication::translate("Config", "Z Zero Dev", nullptr));

    SystemRegister::zStepLengthItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[42] Z Step Length(mm)", nullptr), 
        QCoreApplication::translate("Config", "Z Zero Dev", nullptr));

    SystemRegister::zLimitNumItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[43] Z Limit Num", nullptr), 
        QCoreApplication::translate("Config", "Z limit num", nullptr));

    SystemRegister::zResetEnabledItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[44] Z Reset Enabled", nullptr), 
        QCoreApplication::translate("Config", "Enable z reset", nullptr));

    SystemRegister::zMotorNumItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[45] Z Motor Num", nullptr), 
        QCoreApplication::translate("Config", "Z motor num", nullptr));

    SystemRegister::zMotorCurrentItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[46] Z Motor current(%)", nullptr), 
        QCoreApplication::translate("Config", "Z motor current", nullptr));

    SystemRegister::zStartSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[47] Z Start Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "Z Start Speed", nullptr));

    SystemRegister::zMaxSpeedItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[48] Z Max Speed(mm/s)", nullptr), 
        QCoreApplication::translate("Config", "Z Max Speed", nullptr));

    SystemRegister::zMaxAccelerationItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[49] Z Max Acceleration(mm/s<sup>2</sup>)", nullptr), 
        QCoreApplication::translate("Config", "Z max acceleration", nullptr));

    SystemRegister::zUrgentAccelerationItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[50] Z Urgent Acceleration(mm/s<sup>2</sup>)", nullptr), 
        QCoreApplication::translate("Config", "Z urgent acceleration", nullptr));

    SystemRegister::laserMaxPowerItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[51] Laser Max Power(%)", nullptr),
        QCoreApplication::translate("Config", "Laser Max Power", nullptr));

    SystemRegister::laserMinPowerItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[52] Laser Min Power(%)", nullptr),
        QCoreApplication::translate("Config", "Laser Min Power", nullptr));

    SystemRegister::laserPowerFreqItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[53] Laser Power Frequence", nullptr),
        QCoreApplication::translate("Config", "Laser Power Frequence", nullptr));

    SystemRegister::xPhaseEnabledItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[54] X Phase Enabled", nullptr),
        QCoreApplication::translate("Config", "Enabled X phase", nullptr));

    SystemRegister::yPhaseEnabledItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[55] Y Phase Enabled", nullptr),
        QCoreApplication::translate("Config", "Enabled Y phase", nullptr));

    SystemRegister::zPhaseEnabledItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[56] Z Phase Enabled", nullptr),
        QCoreApplication::translate("Config", "Enabled Z phase", nullptr));

    SystemRegister::deviceOriginItem()->setTitleAndDesc(
        QCoreApplication::translate("Config", "[57] Device Origin", nullptr),
        QCoreApplication::translate("Config", "Device Origin", nullptr));

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
        QCoreApplication::translate("Config", "User Registers", nullptr),
        QCoreApplication::translate("Config", "User Registers", nullptr));

    groupsMap["systemRegister"]->updateTitleAndDesc(
        QCoreApplication::translate("Config", "System Registers", nullptr),
        QCoreApplication::translate("Config", "System Registers", nullptr));
}

void Config::destroy()
{
    save();
    qDeleteAll(groups);
    groups.clear();
    groupsMap.clear();
}
