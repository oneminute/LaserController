#ifndef CONFIG_H
#define CONFIG_H

#include "common/common.h"

#include <QObject>
#include <QString>
#include <QVariant>
#include <QMap>

enum ConfigItemType
{
    CIT_INT,
    CIT_FLOAT,
    CIT_DOUBLE,
    CIT_BOOL,
    CIT_STRING,
    CIT_DATETIME
};


#define DUMMY_STRUCT(PREFIX, NAME, TYPE, DEFAULT_VALUE, ADVANCED) \
    private: \
    public: \
        struct Dummy##PREFIX##NAME: public ConfigItem \
        { \
        public: \
            Dummy##PREFIX##NAME() \
                : ConfigItem(#PREFIX, #NAME, DEFAULT_VALUE, TYPE, ADVANCED) \
            { \
            } \
        }; \
        Dummy##PREFIX##NAME PREFIX##NAME##Stub; \
    public: \
        static void restore##PREFIX##NAME() { items[#PREFIX"/"#NAME].restore(); } \
        static ConfigItem* PREFIX##NAME##Item() { return &items[#PREFIX"/"#NAME]; }

#define CONFIG_ITEM_INT(PREFIX, NAME, DEFAULT_VALUE, ADVANCED) \
    DUMMY_STRUCT(PREFIX, NAME, CIT_INT, DEFAULT_VALUE, ADVANCED) \
    public: \
        static int PREFIX##NAME() { return items[#PREFIX"/"#NAME].value.toInt(); } \
        static int default##PREFIX##NAME() { return items[#PREFIX"/"#NAME].defaultValue.toInt(); } \
        static void set##PREFIX##NAME(int value) \
		{ \
			if (items[#PREFIX"/"#NAME].value != value) \
			{ \
				items[#PREFIX"/"#NAME].value = value; \
				items[#PREFIX"/"#NAME].modified = true; \
			} \
		}

#define CONFIG_ITEM_FLOAT(PREFIX, NAME, DEFAULT_VALUE, ADVANCED) \
    DUMMY_STRUCT(PREFIX, NAME, CIT_FLOAT, DEFAULT_VALUE, ADVANCED) \
    public: \
        static float PREFIX##NAME() { return items[#PREFIX"/"#NAME].value.toFloat(); } \
        static float default##PREFIX##NAME() { return items[#PREFIX"/"#NAME].defaultValue.toFloat(); } \
        static void set##PREFIX##NAME(float value) \
		{ \
			if (items[#PREFIX"/"#NAME].value != value) \
			{ \
				items[#PREFIX"/"#NAME].value = value; \
				items[#PREFIX"/"#NAME].modified = true; \
			} \
		}

#define CONFIG_ITEM_DOUBLE(PREFIX, NAME, DEFAULT_VALUE, ADVANCED) \
    DUMMY_STRUCT(PREFIX, NAME, CIT_FLOAT, DEFAULT_VALUE, ADVANCED) \
    public: \
        static double PREFIX##NAME() { return items[#PREFIX"/"#NAME].value.toDouble(); } \
        static double default##PREFIX##NAME() { return items[#PREFIX"/"#NAME].defaultValue.toDouble(); } \
        static void set##PREFIX##NAME(double value) \
		{ \
			if (items[#PREFIX"/"#NAME].value != value) \
			{ \
				items[#PREFIX"/"#NAME].value = value; \
				items[#PREFIX"/"#NAME].modified = true; \
			} \
		}

#define CONFIG_ITEM_BOOL(PREFIX, NAME, DEFAULT_VALUE, ADVANCED) \
    DUMMY_STRUCT(PREFIX, NAME, CIT_BOOL, DEFAULT_VALUE, ADVANCED) \
    public: \
        static bool PREFIX##NAME() { return items[#PREFIX"/"#NAME].value.toBool(); } \
        static bool default##PREFIX##NAME() { return items[#PREFIX"/"#NAME].defaultValue.toBool(); } \
        static void set##PREFIX##NAME(bool value) \
		{ \
			if (items[#PREFIX"/"#NAME].value != value) \
			{ \
				items[#PREFIX"/"#NAME].value = value; \
				items[#PREFIX"/"#NAME].modified = true; \
			} \
		}

#define CONFIG_ITEM_STRING(PREFIX, NAME, DEFAULT_VALUE, ADVANCED) \
    DUMMY_STRUCT(PREFIX, NAME, CIT_STRING, DEFAULT_VALUE, ADVANCED) \
    public: \
        static QString PREFIX##NAME() { return items[#PREFIX"/"#NAME].value.toString(); } \
        static QString default##PREFIX##NAME() { return items[#PREFIX"/"#NAME].defaultValue.toString(); } \
        static void set##PREFIX##NAME(const QString& value) \
		{ \
			if (items[#PREFIX"/"#NAME].value != value) \
			{ \
				items[#PREFIX"/"#NAME].value = value; \
				items[#PREFIX"/"#NAME].modified = true; \
			} \
		}


class Config
{
public:
    struct ConfigItem
    {
    public:
        ConfigItem(const QString& _prefix = QObject::tr("General"),
            const QString& _name = "",
            const QVariant& _defaultValue = QVariant(),
            ConfigItemType _type = CIT_INT,
            const bool _advanced = false);
        QString prefix;
        QString title;
        QString description;
        QString name;
        QString key;
        QVariant value;
        QVariant defaultValue;
        ConfigItemType type;
        bool advanced;
        bool modified;

        void restore()
        {
            value = defaultValue;
            modified = true;
        }

        QString toString()
        {
            return QString("prefix: %1, name: %2, value: %3, default value:%4, type: %5, description: %6")
                .arg(prefix).arg(name).arg(value.toString()).arg(defaultValue.toString()).arg(type).arg(description);
        }
    };

    CONFIG_ITEM_INT(General, Language, 25, false)
	CONFIG_ITEM_INT(General, Unit, (int)SU_MM, false)

    CONFIG_ITEM_INT(Layers, MaxLayersCount, 16, false)

    CONFIG_ITEM_INT(UI, OperationButtonIconSize, 32, false)
    CONFIG_ITEM_INT(UI, OperationButtonWidth, 60, false)
    CONFIG_ITEM_INT(UI, OperationButtonHeight, 60, false)
    CONFIG_ITEM_BOOL(UI, OperationButtonShowText, true, false)

    CONFIG_ITEM_INT(UI, ToolButtonSize, 32, false)

    CONFIG_ITEM_INT(UI, ColorButtonWidth, 30, false)
    CONFIG_ITEM_INT(UI, ColorButtonHeight, 30, false)

	CONFIG_ITEM_INT(CuttingLayer, MinSpeed, 15, false)
	CONFIG_ITEM_INT(CuttingLayer, RunSpeed, 60, false)
	CONFIG_ITEM_INT(CuttingLayer, LaserPower, 80, false)
	CONFIG_ITEM_INT(CuttingLayer, MinSpeedPower, 700, false)
	CONFIG_ITEM_INT(CuttingLayer, RunSpeedPower, 1000, false)

	CONFIG_ITEM_INT(EngravingLayer, MinSpeed, 60, false)
	CONFIG_ITEM_INT(EngravingLayer, RunSpeed, 300, false)
	CONFIG_ITEM_INT(EngravingLayer, LaserPower, 115, false)
	CONFIG_ITEM_INT(EngravingLayer, MinSpeedPower, 0, false)
	CONFIG_ITEM_INT(EngravingLayer, RunSpeedPower, 900, false)
	CONFIG_ITEM_BOOL(EngravingLayer, UseHalftone, true, false)
	CONFIG_ITEM_INT(EngravingLayer, LPI, 600, false)
	CONFIG_ITEM_INT(EngravingLayer, DPI, 600, false)

    CONFIG_ITEM_INT(OptimizePath, MaxAnts, 100, false)
    CONFIG_ITEM_INT(OptimizePath, MaxIterations, 500, false)
    CONFIG_ITEM_INT(OptimizePath, MaxTraverseCount, 2000, false)
    CONFIG_ITEM_DOUBLE(OptimizePath, VolatileRate, 0.65, false)
    CONFIG_ITEM_BOOL(OptimizePath, UseGreedyAlgorithm, true, false)
    CONFIG_ITEM_INT(OptimizePath, MaxStartingPoints, 8, false)
    CONFIG_ITEM_INT(OptimizePath, MaxStartingPointAnglesDiff, 45, false)
    //CONFIG_ITEM_DOUBLE(OptimizePath, MinStartingPointsInterval, 8, "Min interval between two starting points.")

    CONFIG_ITEM_DOUBLE(PltUtils, MaxAnglesDiff, 5.0, false)
    CONFIG_ITEM_DOUBLE(PltUtils, MaxIntervalDistance, 10.0, false)

    CONFIG_ITEM_BOOL(Device, AutoConnectFirst, true, false)

private:
    Config();
    ~Config();

public:
    static void load();
    static void loadTitlesAndDescriptions();
    static void save();
    static void restore();
    static QString configFilePath();
    static bool isModified();

private:
    static QMap<QString, ConfigItem> items;
    static Config config;
};

#endif // CONFIG_H