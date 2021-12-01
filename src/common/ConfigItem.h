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
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled)

public:
    typedef std::function<QWidget*(ConfigItem*)> CreateWidgetHook;
    typedef std::function<void(QWidget*, ConfigItem*, InputWidgetWrapper*)> WidgetInitializeHook;
    typedef std::function<void(ConfigItem*)> DestroyHook;
    typedef std::function<QJsonObject(const ConfigItem*)> ToJsonHook;
    typedef std::function<void(QVariant& value, QVariant& defaultValue, const QJsonObject&, ConfigItem*)> FromJsonHook;
    typedef std::function<QVariant(QWidget* widget, const QVariant& value)> WidgetValueHook;
    typedef std::function<void(QWidget*, ConfigItem*)> RetranslateHook;

    explicit ConfigItem(const QString& name
        , ConfigItemGroup* group
        , const QVariant& value
        , DataType dataType = DT_INT
        , bool advanced = false
        , bool visible = true
        , StoreStrategy storeStrategy = StoreStrategy::SS_AS_IS
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

    bool needRelaunch() const;

    bool isVisible() const;
    void setVisible(bool visible);

    bool isEnabled() const;
    void setEnabled(bool enabled);

    bool exportable() const;
    void setExportable(bool exportable);

    StoreStrategy storeStrategy() const;
    void setStoreStrategy(StoreStrategy type);

    QVariant value() const;
    QVariant oldValue() const;

    QVariant defaultValue() const;
    void setDefaultValue(const QVariant& value);

    QVariant systemDefaultValue() const;
    void setSystemDefaultValue(const QVariant& value);

    QVariant lastValue() const;
    void setLastValue(const QVariant& value);

    bool isDirty() const;
    bool isModified() const;

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

    void setValue(const QVariant& value, StoreStrategy strategy, void* senderPtr);

protected:
    InputWidgetWrapper* bindWidget(QWidget* widget, StoreStrategy ss);

    /// <summary>
    /// 绑定的控件在初始化时的回调函数
    /// </summary>
    /// <returns></returns>
    WidgetInitializeHook widgetInitializeHook();
    void setWidgetInitializeHook(WidgetInitializeHook fn);
    void doInitWidget(QWidget* widget, InputWidgetWrapper* wrapper);

    /*/// <summary>
    /// 当填充控件值时的回调函数
    /// </summary>
    /// <returns></returns>
    ValueHook valueToWidgetHook();
    void setValueToWidgetHook(ValueHook fn);
    QVariant doValueToWidgetHook(const QVariant& value) const;*/

    /// <summary>
    /// 当用控件的值改写选项值时的回调函数
    /// </summary>
    /// <returns></returns>
    WidgetValueHook valueFromWidgetHook();
    void setValueFromWidgetHook(WidgetValueHook fn);
    QVariant doValueFromWidgetHook(QWidget* widget, const QVariant& value);

    /// <summary>
    /// 当创建控件时的回调函数
    /// </summary>
    /// <returns></returns>
    CreateWidgetHook createWidgetHook();
    void setCreateWidgetHook(CreateWidgetHook fn);
    QWidget* doCreateWidgetHook();

    /// <summary>
    /// 销毁控件时的回调函数
    /// </summary>
    /// <returns></returns>
    DestroyHook destroyHook();
    void setDestroyHook(DestroyHook fn);
    void doDestroyHook();

    /// <summary>
    /// 生成Json时的回调函数
    /// </summary>
    /// <returns></returns>
    ToJsonHook toJsonHook();
    void setToJsonHook(ToJsonHook fn);
    QJsonObject doToJsonHook() const;

    /// <summary>
    /// 从json解析时的回调函数
    /// </summary>
    /// <returns></returns>
    FromJsonHook fromJsonHook();
    void setFromJsonHook(FromJsonHook fn);
    void doFromJsonHook(const QJsonObject& json);

    WidgetValueHook updateWidgetValueHook();
    void setUpdateWidgetValueHook(WidgetValueHook fn);
    bool doUpdateWidgetValueHook(QWidget* widget, const QVariant& value);

    RetranslateHook retranslateHook();
    void setRetranslateHook(RetranslateHook fn);
    void doRetranslateHook(QWidget* widget);

    const QList<QWidget*>& boundedWidgets() const;

    void reset();
    void restoreToDefault();
    void restoreToSystemDefault();

    void apply();
    void applyToDefault();
    bool confirm(const QVariant& value);
    void loadValue(const QVariant& value);
    void clearModified();

signals:
    void visibleChanged(bool value);
    void defaultValueChanged(const QVariant& value);
    void modifiedChanged(bool modified);
    void enabledChanged(bool enabled);

    void valueChanged(const QVariant& value, void* senderPtr);
    void dirtyValueChanged(const QVariant& value, void* senderPtr);
    void lazyValueChanged(const QVariant& value, void* snederPtr);

private:
    QScopedPointer<ConfigItemPrivate> m_ptr;

    Q_DECLARE_PRIVATE_D(m_ptr, ConfigItem)
    Q_DISABLE_COPY(ConfigItem)

    friend class Config;
    friend class ConfigItemGroup;
    friend class ConfigDialog;
    friend class InputWidgetWrapper;
    friend class LaserControllerWindow;
    friend class LaserDevice;
};

QDebug operator<<(QDebug debug, const ConfigItem & item);

#endif // CONFIGITEM_H