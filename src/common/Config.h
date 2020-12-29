#ifndef CONFIG_H
#define CONFIG_H

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

#define DUMMY_STRUCT(PREFIX, NAME, TYPE, DEFAULT_VALUE, DESCRIPTION) \
    public: \
        struct Dummy##PREFIX##NAME: public ConfigItem \
        { \
        public: \
            Dummy##PREFIX##NAME() \
                : ConfigItem(#PREFIX, #NAME, QObject::tr(DESCRIPTION), DEFAULT_VALUE, TYPE) \
            {} \
        }; \
        Dummy##PREFIX##NAME PREFIX##NAME##Stub; \
    public: \
        static void restore##PREFIX##NAME() { items[#PREFIX"/"#NAME].restore(); } \
        static QString PREFIX##NAME##Description() { return items[#PREFIX"/"#NAME].description; } \
        static ConfigItem* PREFIX##NAME##Item() { return &items[#PREFIX"/"#NAME]; }

#define CONFIG_ITEM_INT(PREFIX, NAME, DEFAULT_VALUE, DESCRIPTION) \
    DUMMY_STRUCT(PREFIX, NAME, CIT_INT, DEFAULT_VALUE, DESCRIPTION) \
    public: \
        static int PREFIX##NAME() { return items[#PREFIX"/"#NAME].value.toInt(); } \
        static int default##PREFIX##NAME() { return items[#PREFIX"/"#NAME].defaultValue.toInt(); } \
        static void setPREFIX##NAME(int value) { items[#PREFIX"/"#NAME].value = value; items[#PREFIX"/"#NAME].modified = true; }

#define CONFIG_ITEM_BOOL(PREFIX, NAME, DEFAULT_VALUE, DESCRIPTION) \
    DUMMY_STRUCT(PREFIX, NAME, CIT_BOOL, DEFAULT_VALUE, DESCRIPTION) \
    public: \
        static bool PREFIX##NAME() { return items[#PREFIX"/"#NAME].value.toBool(); } \
        static bool default##PREFIX##NAME() { return items[#PREFIX"/"#NAME].defaultValue.toBool(); } \
        static void setPREFIX##NAME(bool value) { items[#PREFIX"/"#NAME].value = value; items[#PREFIX"/"#NAME].modified = true; }

#define CONFIG_ITEM_STRING(PREFIX, NAME, DEFAULT_VALUE, DESCRIPTION) \
    DUMMY_STRUCT(PREFIX, NAME, CIT_STRING, DEFAULT_VALUE, DESCRIPTION) \
    public: \
        static QString PREFIX##NAME() { return items[#PREFIX"/"#NAME].value.toString(); } \
        static QString default##PREFIX##NAME() { return items[#PREFIX"/"#NAME].defaultValue.toString(); } \
        static void setPREFIX##NAME(const QString& value) { items[#PREFIX"/"#NAME].value = value; items[#PREFIX"/"#NAME].modified = true; }

class Config
{
public:
    struct ConfigItem
    {
    public:
        ConfigItem(const QString& _prefix = "General",
            const QString& _name = "",
            const QString& _description = "",
            const QVariant& _defaultValue = QVariant(),
            ConfigItemType _type = CIT_INT);
        QString prefix;
        QString name;
        QString key;
        QString description;
        QVariant value;
        QVariant defaultValue;
        ConfigItemType type;
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

    CONFIG_ITEM_STRING(General, Language, "English", "Language for both UI and Business.")

    CONFIG_ITEM_INT(Layers, MaxLayersCount, 16, "Max layers count.")

    CONFIG_ITEM_INT(UI, OperationButtonIconSize, 32, "Size of operation buttons' icons.")
    CONFIG_ITEM_INT(UI, OperationButtonWidth, 60, "Width of operation buttons.")
    CONFIG_ITEM_INT(UI, OperationButtonHeight, 60, "Height of operation buttons.")
    CONFIG_ITEM_BOOL(UI, OperationButtonShowText, true, "Whether to show text of operation button or not.")

    CONFIG_ITEM_INT(UI, ToolButtonSize, 32, "Size of tool buttons")

    CONFIG_ITEM_INT(UI, ColorButtonWidth, 30, "Width of the color buttons.")
    CONFIG_ITEM_INT(UI, ColorButtonHeight, 30, "Height of the color buttons.")

private:
    Config();

public:
    static void load();
    static void save();
    static void restore();
    static QString configFilePath();
    static bool isModified();

private:
    static QMap<QString, ConfigItem> items;
    static Config config;
};

#endif // CONFIG_H