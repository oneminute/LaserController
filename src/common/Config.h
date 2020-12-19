#ifndef CONFIG_H
#define CONFIG_H

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
    private: \
        struct Dummy##PREFIX##NAME: ConfigItem \
        { \
        public: \
            Dummy##PREFIX##NAME() \
                : ConfigItem(#PREFIX, #NAME, DESCRIPTION, DEFAULT_VALUE, TYPE) \
            {} \
        }; \
        Dummy##PREFIX##NAME PREFIX##NAME##Item; \
    public: \
        static void restore##PREFIX##NAME() { items[#PREFIX"/"#NAME].restore(); }

#define CONFIG_ITEM_INT(PREFIX, NAME, DEFAULT_VALUE, DESCRIPTION) \
    DUMMY_STRUCT(PREFIX, NAME, CIT_INT, DEFAULT_VALUE, DESCRIPTION) \
    public: \
        static int PREFIX##NAME() { return items[#PREFIX"/"#NAME].value.toInt(); } \
        static int default##PREFIX##NAME() { return items[#PREFIX"/"#NAME].defaultValue.toInt(); } \
        static void setPREFIX##NAME(int value) { items[#PREFIX"/"#NAME].value = value; items[#PREFIX"/"#NAME].modified = true; } \

#define CONFIG_ITEM_STRING(PREFIX, NAME, DEFAULT_VALUE, DESCRIPTION) \
    DUMMY_STRUCT(PREFIX, NAME, CIT_STRING, DEFAULT_VALUE, DESCRIPTION) \
    public: \
        static QString PREFIX##NAME() { return items[#PREFIX"/"#NAME].value.toString(); } \
        static QString default##PREFIX##NAME() { return items[#PREFIX"/"#NAME].defaultValue.toString(); } \
        static void setPREFIX##NAME(const QString& value) { items[#PREFIX"/"#NAME].value = value; items[#PREFIX"/"#NAME].modified = true; } \

class Config
{
private:
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

    CONFIG_ITEM_INT(Layers, MaxLayersCount, 16, "Max layers count.")
    CONFIG_ITEM_STRING(General, Language, "English", "Language for both UI and Business.")

public:
    static void load();
    static void save();
    static void restore();
    static QString configFilePath();

private:
    static QMap<QString, ConfigItem> items;
    static Config config;
};

#endif // CONFIG_H