#ifndef CONFIGITEM_H
#define CONFIGITEM_H

#include <QDebug>
#include <QJsonObject>
#include <QObject>

#include "common/common.h"

class ConfigItemGroup;
class InputWidgetWrapper;

class ConfigItemPrivate;
class ConfigItem: public QObject
{
    Q_OBJECT
public:
    typedef std::function<QWidget*(ConfigItem*)> CreateWidgetHook;
    typedef std::function<void(QWidget*, ConfigItem*, InputWidgetWrapper*)> WidgetInitializeHook;
    typedef std::function<QVariant(const QVariant&)> ValueHook;
    typedef std::function<void(ConfigItem*)> DestroyHook;
    typedef std::function<QJsonObject(const ConfigItem*)> ToJsonHook;
    typedef std::function<void(QVariant& value, QVariant& defaultValue, const QJsonObject&, ConfigItem*)> FromJsonHook;
    typedef std::function<void(ConfigItem*)> ResetHook;
    typedef std::function<void(ConfigItem*)> RestoreHook;
    typedef std::function<void(QWidget* widget, const QVariant& value)> UpdateWidgetValueHook;

    explicit ConfigItem(const QString& name
        , ConfigItemGroup* group
        //, const QString& title
        //, const QString& description
        , const QVariant& value
        , DataType dataType = DT_INT
        , bool advanced = false
        , bool visible = true
        , StoreStrategy storeStrategy = StoreStrategy::SS_CONFIRMED
    );
    ~ConfigItem();

    QString fullName() const;
    QString name() const;
    ConfigItemGroup* group() const;
    QString groupName() const;
    QString title() const;
    void setTitle(const QString& title);
    QString description() const;
    void setDescription(const QString& description);
    void setTitleAndDesc(const QString& title, const QString& desc);

    bool isAdvanced() const;
    void setAdvanced(bool advanced);

    bool isVisible() const;
    void setVisible(bool visible);

    bool isEnabled() const;
    void setEnabled(bool enabled);

    bool exportable() const;
    void setExportable(bool exportable);

    bool readOnly() const;
    void setReadOnly(bool readOnly = true);

    bool writeOnly() const;
    void setWriteOnly(bool writeOnly = true);

    StoreStrategy storeStrategy() const;
    void setStoreStrategy(StoreStrategy type);

    QVariant value() const;

    QVariant defaultValue() const;
    void setDefaultValue(const QVariant& value);

    QVariant systemDefaultValue() const;
    void setSystemDefaultValue(const QVariant& value);

    QVariant lastValue() const;
    void setLastValue(const QVariant& value);

    bool isModified() const;

    InputWidgetWrapper* bindWidget(QWidget* widget);

    QString toString() const;
    QJsonObject toJson() const;
    void fromJson(const QJsonObject& jsonObject);

    DataType dataType() const;

    void setInputWidgetType(InputWidgetType widgetType);
    InputWidgetType inputWidgetType() const;

    QMap<QString, QVariant>& inputWidgetProperties();
    void setInputWidgetProperty(const QString& key, const QVariant& value);

    QVariantMap& extraProperties();
    void setExtraProperty(const QString& key, const QVariant& value);
    QVariant& extraProperty(const QString& key);

    WidgetInitializeHook widgetInitializeHook();
    void setWidgetInitializeHook(WidgetInitializeHook fn);
    void doInitWidget(QWidget* widget, InputWidgetWrapper* wrapper);

    ValueHook valueToWidgetHook();
    void setValueToWidgetHook(ValueHook fn);
    QVariant doValueToWidgetHook(const QVariant& value) const;

    ValueHook valueFromWidgetHook();
    void setValueFromWidgetHook(ValueHook fn);
    QVariant doValueFromWidgetHook(const QVariant& value);

    CreateWidgetHook createWidgetHook();
    void setCreateWidgetHook(CreateWidgetHook fn);
    QWidget* doCreateWidgetHook();

    DestroyHook destroyHook();
    void setDestroyHook(DestroyHook fn);
    void doDestroyHook();

    ToJsonHook toJsonHook();
    void setToJsonHook(ToJsonHook fn);
    QJsonObject doToJsonHook() const;

    FromJsonHook fromJsonHook();
    void setFromJsonHook(FromJsonHook fn);
    void doFromJsonHook(const QJsonObject& json);

    ResetHook resetHook();
    void setResetHook(ResetHook fn);
    void doResetHook();

    RestoreHook restoreHook();
    void setRestoreHook(RestoreHook fn);
    void doRestoreHook();

    UpdateWidgetValueHook updateWidgetValueHook();
    void setUpdateWidgetValueHook(UpdateWidgetValueHook fn);
    bool doUpdateWidgetValueHook(QWidget* widget, const QVariant& value);

    const QList<QWidget*>& boundedWidgets() const;

    ModifiedBy modifiedBy() const;

    void setValue(const QVariant& value, ModifiedBy modifiedBy = MB_Manual);

public slots:
    void reset();
    void restore();
    void restoreSystem();
    void confirm();

protected slots:
    void onRegisterLoaded(const QVariant& value);

signals:
    void visibleChanged(bool value);
    void valueChanged(const QVariant& value, ModifiedBy modifiedBy);
    void defaultValueChanged(const QVariant& value);
    void modifiedChanged(bool modified);
    void enabledChanged(bool enabled);

private:
    QScopedPointer<ConfigItemPrivate> m_ptr;

    Q_DECLARE_PRIVATE_D(m_ptr, ConfigItem)
    Q_DISABLE_COPY(ConfigItem)

    friend class Config;
};

QDebug operator<<(QDebug debug, const ConfigItem & item);

#endif // CONFIGITEM_H