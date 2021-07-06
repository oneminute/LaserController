#include "Config.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

#include "ConfigItem.h"
#include "laser/LaserRegister.h"
#include "widget/InputWidgetWrapper.h"

QList<ConfigItemGroup*> Config::groups;
QMap<QString, ConfigItemGroup*> Config::groupsMap;

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

void Config::save()
{
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
    /*for (QMap<QString, ConfigItem*>::ConstIterator i = items.constBegin(); i != items.constEnd(); i++)
    {
        if (i.value()->isModified())
        {
            isModified = true;
            break;
        }
    }*/
    return isModified;
}

void Config::loadGeneralItems()
{
    ConfigItemGroup* group = new Config::General;

    ConfigItem* language = group->addConfigItem(
        "language"
        , tr("Language")
        , tr("Language for both UI and Business.")
        , 25
    );
    language->setInputWidgetType(IWT_ComboBox);
    language->setCreateWidgetFunction(
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
    unit->setCreateWidgetFunction(
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

    group->addConfigItem(
        "autoConnectFirst",
        tr("Auto Connect First"),
        tr("Auto connect to first com port when found multiple laser devices."),
        true,
        DT_BOOL
    );
}

QList<ConfigItemGroup*> Config::getGroups()
{
    return groups;
}
