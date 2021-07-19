#ifndef CONFIGITEM_H
#define CONFIGITEM_H

#include <QDebug>
#include <QJsonObject>
#include <QObject>

#include "common/common.h"
#include "laser/LaserRegister.h"

class Config;
class ConfigItemGroup;
class InputWidgetWrapper;
class LaserRegister;

class ConfigItemPrivate;
class ConfigItem: public QObject
{
    Q_OBJECT
public:
    typedef void (*WidgetInitializeHook)(QWidget*, ConfigItem*);
    typedef QVariant (*ValueHook)(const QVariant&);

    explicit ConfigItem(const QString& name
        , ConfigItemGroup* group
        , const QString& title
        , const QString& description
        , const QVariant& value
        , DataType dataType = DT_INT
        , bool advanced = false
        , bool visible = true
        , StoreStrategy storeType = StoreStrategy::SS_CONFIRMED
    );
    ~ConfigItem();

    QString fullName() const;
    QString name() const;
    ConfigItemGroup* group() const;
    QString groupName() const;
    QString title() const;
    QString description() const;

    bool isAdvanced() const;
    void setAdvanced(bool advanced);

    bool isVisible() const;
    void setVisible(bool visible);

    bool readOnly() const;
    void setReadOnly(bool readOnly = true);

    StoreStrategy storeType() const;
    void setStoreType(StoreStrategy type);

    QVariant value() const;

    QVariant defaultValue() const;
    void setDefaultValue(const QVariant& value);

    QVariant systemDefaultValue() const;
    void setSystemDefaultValue(const QVariant& value);

    QVariant lastValue() const;
    void setLastValue(const QVariant& value);

    bool isDirty() const;
    void setDirty(bool dirty);

    bool isModified() const;

    InputWidgetWrapper* createInputWidgetWrapper(QWidget* widget);

    QString toString() const;
    QJsonObject toJson() const;
    void fromJson(const QJsonObject& jsonObject);

    QString toRegisterString() const;
    LaserRegister::RegisterPair keyValuePair() const;

    DataType dataType() const;

    void setInputWidgetType(InputWidgetType widgetType);
    InputWidgetType inputWidgetType() const;

    QMap<QString, QVariant>& inputWidgetProperties();
    void setInputWidgetProperty(const QString& key, const QVariant& value);

    WidgetInitializeHook widgetInitializeHook();
    void setWidgetInitializeHook(WidgetInitializeHook fn);

    ValueHook loadDataHook();
    void setLoadDataHook(ValueHook fn);
    QVariant doLoadDataHook(const QVariant& value) const;

    ValueHook saveDataHook();
    void setSaveDataHook(ValueHook fn);
    QVariant doSaveDataHook(const QVariant& value);

    void initWidget(QWidget* widget);

    LaserRegister* laserRegister() const;
    void bindLaserRegister(int addr, bool isSystem = true, StoreStrategy storeStrategy = SS_CONFIRMED);
    bool hasRegister() const;

public slots:
    void setValue(const QVariant& value);
    void fromWidget(const QVariant& value);
    void loadValue(const QVariant& value);
    void reset();
    void doModify();
    void restore();
    void restoreSystem();

protected:
    bool modifyValue(const QVariant& value);
    void setModified();
    void setName(const QString& name);
    void setDescription(const QString& description);

    void setValueDirectly(const QVariant& value);
    void setValueConfirmed(const QVariant& value);
    void setValueLazy(const QVariant& value);

signals:
    void visibleChanged(bool value);
    void valueChanged(const QVariant& value);
    void widgetValueChanged(const QVariant& value);
    void defaultValueChanged(const QVariant& value);
    void modifiedChanged(bool modified);

private:
    QScopedPointer<ConfigItemPrivate> m_ptr;

    Q_DECLARE_PRIVATE_D(m_ptr, ConfigItem)
    Q_DISABLE_COPY(ConfigItem)

    friend class Config;
};

QDebug operator<<(QDebug debug, const ConfigItem & item);

#endif // CONFIGITEM_H