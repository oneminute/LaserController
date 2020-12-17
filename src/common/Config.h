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
        Dummy##PREFIX##NAME PREFIX##NAME##Item;

#define CONFIG_ITEM_INT(PREFIX, NAME, DEFAULT_VALUE, DESCRIPTION) \
    DUMMY_STRUCT(PREFIX, NAME, CIT_INT, DEFAULT_VALUE, DESCRIPTION) \
    public: \
        static int PREFIX##NAME() { return items[#PREFIX"/"#NAME].value.toInt(); } \
        static int default##PREFIX##NAME() { return items[#PREFIX"/"#NAME].defaultValue.toInt(); } \
        static void setPREFIX##NAME(int value) { items[#PREFIX"/"#NAME].value = value; items[#PREFIX"/"#NAME].modified = true; } \
        static void restore##PREFIX##NAME() { items[#PREFIX"/"#NAME].restore(); }

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
    };

    CONFIG_ITEM_INT(Layers, MaxLayersCount, 16, "Max layers count.")

public:
    void load();
    void save();
    void restore();
    QString path();

private:
    static QMap<QString, ConfigItem> items;
    static Config config;
};

#endif // CONFIG_H