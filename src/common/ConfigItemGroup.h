#ifndef CONFIGITEMGROUP_H
#define CONFIGITEMGROUP_H

#include <QJsonObject>
#include <QObject>
#include "common.h"

class Config;
class ConfigItem;

class ConfigItemGroupPrivate;
class ConfigItemGroup: public QObject
{
    Q_OBJECT
public:
    explicit ConfigItemGroup(const QString& name, const QString& title, const QString& description, QObject* parent = nullptr);

    ~ConfigItemGroup();

    QString name() const;

    QString title() const;

    QString description() const;

    void addConfigItem(ConfigItem* item);

    ConfigItem* addConfigItem(const QString& name
        , const QString& title
        , const QString& description
        , const QVariant& value
        , DataType dataType = DT_INT
        , bool advanced = false
        , bool visible = true
        , StoreStrategy storeType = StoreStrategy::SS_CONFIRMED
    );

    QList<ConfigItem*>& items();
    ConfigItem* configItem(const QString& name);

    QJsonObject toJson() const;
    void fromJson(const QJsonObject& jsonObject);

    bool isModified() const;

    void doModify();

protected:

private:
    QScopedPointer<ConfigItemGroupPrivate> m_ptr;

    Q_DECLARE_PRIVATE_D(m_ptr, ConfigItemGroup)
    Q_DISABLE_COPY(ConfigItemGroup)

    friend class Config;
};

#endif // CONFIGITEMGROUP_H