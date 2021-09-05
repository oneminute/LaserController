#include "ConfigItem.h"
#include "ConfigItemGroup.h"
#include "widget/InputWidgetWrapper.h"

#include <QDateTime>

class ConfigItemPrivate
{
    Q_DECLARE_PUBLIC(ConfigItem)
public:
    ConfigItemPrivate(ConfigItem* ptr)
        : q_ptr(ptr)
        , group(nullptr)
        , advanced(false)
        , visible(true)
        , enabled(true)
        , exportable(true)
        , readOnly(false)
        , writeOnly(false)
        //, modified(false)
        , inputWidgetType(IWT_EditSlider)
        , modifiedBy(MB_Manual)
        , widgetInitializeHook(nullptr)
        , valueToWidgetHook(nullptr)
        , valueFromWidgetHook(nullptr)
        , createWidgetHook(nullptr)
        , destroyHook(nullptr)
        , toJsonHook(nullptr)
        , fromJsonHook(nullptr)
        , resetHook(nullptr)
        , restoreHook(nullptr)
        , updateWidgetValueHook(nullptr)
    {
        
    }

    ConfigItem* q_ptr;

    /// <summary>
    /// 选项名
    /// </summary>
    QString name;

    /// <summary>
    /// 选项的分组对象
    /// </summary>
    ConfigItemGroup* group;

    /// <summary>
    /// 标题
    /// </summary>
    QString title;

    /// <summary>
    /// 描述
    /// </summary>
    QString description;

    /// <summary>
    /// 是否为高级选项
    /// </summary>
    bool advanced;

    /// <summary>
    /// 是否在选项窗口中出现
    /// </summary>
    bool visible;

    /// <summary>
    /// 是否启用
    /// </summary>
    bool enabled;

    /// <summary>
    /// 是否可导出
    /// </summary>
    bool exportable;

    /// <summary>
    /// 是否为只读选项
    /// </summary>
    bool readOnly;

    /// <summary>
    /// 只写
    /// </summary>
    bool writeOnly;

    /// <summary>
    /// 保存方式
    /// </summary>
    StoreStrategy storeStrategy;

    /// <summary>
    /// 选项值
    /// </summary>
    QVariant value;

    /// <summary>
    /// 默认值
    /// </summary>
    QVariant defaultValue;

    /// <summary>
    /// 系统默认值，该值不可更改
    /// </summary>
    QVariant systemDefaultValue;

    /// <summary>
    /// 上一次的值
    /// </summary>
    QVariant lastValue;

    /// <summary>
    /// 临时值
    /// </summary>
    QVariant dirtyValue;

    /// <summary>
    /// 是否已经被修改
    /// </summary>
    //bool modified;

    /// <summary>
    /// 数据类型
    /// </summary>
    DataType dataType;

    /// <summary>
    /// 建议的输入控件
    /// </summary>
    InputWidgetType inputWidgetType;

    /// <summary>
    /// 建议的输入控件的属性值
    /// </summary>
    QMap<QString, QVariant> inputWidgetProperties;

    QList<QWidget*> widgets;

    ModifiedBy modifiedBy;

    ConfigItem::WidgetInitializeHook widgetInitializeHook;
    ConfigItem::ValueHook valueToWidgetHook;
    ConfigItem::ValueHook valueFromWidgetHook;
    ConfigItem::CreateWidgetHook createWidgetHook;
    ConfigItem::DestroyHook destroyHook;
    ConfigItem::ToJsonHook toJsonHook;
    ConfigItem::FromJsonHook fromJsonHook;
    ConfigItem::ResetHook resetHook;
    ConfigItem::RestoreHook restoreHook;
    ConfigItem::UpdateWidgetValueHook updateWidgetValueHook;
};

ConfigItem::ConfigItem(
    const QString& name
    , ConfigItemGroup* group
    , const QString& title
    , const QString& description
    , const QVariant& value
    , DataType dataType
    , bool advanced
    , bool visible
    , StoreStrategy storeStrategy)
    : QObject(group)
    , m_ptr(new ConfigItemPrivate(this))
{
    Q_D(ConfigItem);
    d->name = name;
    d->group = group;
    d->title = title;
    d->description = description;
    d->value = value;
    d->defaultValue = value;
    d->systemDefaultValue = value;
    d->dataType = dataType;
    d->advanced = advanced;
    d->visible = visible;
    d->storeStrategy = storeStrategy;
    d->modifiedBy = MB_Manual;

    switch (d->dataType)
    {
    case DT_INT:
        d->inputWidgetType = IWT_EditSlider;
        break;
    case DT_BOOL:
        d->inputWidgetType = IWT_CheckBox;
        break;
    case DT_FLOAT:
    case DT_DOUBLE:
    case DT_REAL:
        d->inputWidgetType = IWT_FloatEditSlider;
        break;
    case DT_STRING:
        d->inputWidgetType = IWT_LineEdit;
        break;
    case DT_DATETIME:
        d->inputWidgetType = IWT_DateTimeEdit;
        break;
    default:
        d->inputWidgetType = IWT_Custom;
    }
}

ConfigItem::~ConfigItem()
{
    doDestroyHook();
}

QString ConfigItem::fullName() const
{
    Q_D(const ConfigItem);
    return QString("%1/%2").arg(group()->name()).arg(name());
}

QString ConfigItem::name() const
{
    Q_D(const ConfigItem);
    return d->name;
}

ConfigItemGroup* ConfigItem::group() const
{
    Q_D(const ConfigItem);
    return d->group;
}

QString ConfigItem::groupName() const
{
    Q_D(const ConfigItem);
    return d->group->name();
}

QString ConfigItem::title() const
{
    Q_D(const ConfigItem);
    return d->title;
}

QString ConfigItem::description() const
{
    Q_D(const ConfigItem);
    return d->description;
}

bool ConfigItem::isAdvanced() const
{
    Q_D(const ConfigItem);
    return d->advanced;
}

void ConfigItem::setAdvanced(bool advanced)
{
    Q_D(ConfigItem);
    d->advanced = advanced;
}

bool ConfigItem::isVisible() const
{
    Q_D(const ConfigItem);
    return d->visible;
}

void ConfigItem::setVisible(bool visible)
{
    Q_D(ConfigItem);
    bool changed = d->visible != visible;
    d->visible = visible;
    if (changed) 
        emit visibleChanged(visible);
}

bool ConfigItem::isEnabled() const
{
    Q_D(const ConfigItem);
    return d->enabled;
}

void ConfigItem::setEnabled(bool enabled)
{
    Q_D(ConfigItem);
    d->enabled = enabled;
    emit enabledChanged(enabled);
}

bool ConfigItem::exportable() const
{
    Q_D(const ConfigItem);
    return d->exportable;
}

void ConfigItem::setExportable(bool exportable)
{
    Q_D(ConfigItem);
    d->exportable = exportable;
}

bool ConfigItem::readOnly() const
{
    Q_D(const ConfigItem);
    return d->readOnly;
}

void ConfigItem::setReadOnly(bool readOnly)
{
    Q_D(ConfigItem);
    d->readOnly = readOnly;
}

bool ConfigItem::writeOnly() const
{
    Q_D(const ConfigItem);
    return d->writeOnly;
}

void ConfigItem::setWriteOnly(bool writeOnly)
{
    Q_D(ConfigItem);
    d->writeOnly = writeOnly;
}

StoreStrategy ConfigItem::storeStrategy() const
{
    Q_D(const ConfigItem);
    return d->storeStrategy;
}

void ConfigItem::setStoreStrategy(StoreStrategy type)
{
    Q_D(ConfigItem);
    d->storeStrategy = type;
}

QVariant ConfigItem::value() const
{
    Q_D(const ConfigItem);
    if (isModified())
    {
        return d->dirtyValue;
    }
    else
    {
        return d->value;
    }
}

QVariant ConfigItem::defaultValue() const
{
    Q_D(const ConfigItem);
    return d->defaultValue;
}

void ConfigItem::setDefaultValue(const QVariant& value)
{
    Q_D(ConfigItem);
    bool changed = value != d->defaultValue;
    d->defaultValue = value;
    if (changed)
        emit defaultValueChanged(value);
}

QVariant ConfigItem::systemDefaultValue() const
{
    Q_D(const ConfigItem);
    return d->systemDefaultValue;
}

void ConfigItem::setSystemDefaultValue(const QVariant& value)
{
    Q_D(ConfigItem);
    d->systemDefaultValue = value;
}

QVariant ConfigItem::lastValue() const
{
    Q_D(const ConfigItem);
    return d->lastValue;
}

void ConfigItem::setLastValue(const QVariant& value)
{
    Q_D(ConfigItem);
    d->lastValue = value;
}

bool ConfigItem::isModified() const
{
    Q_D(const ConfigItem);
    //return d->modified;
    return d->dirtyValue != d->value;
}

InputWidgetWrapper* ConfigItem::bindWidget(QWidget* widget)
{
    Q_D(ConfigItem);
    InputWidgetWrapper* wrapper = new InputWidgetWrapper(widget, this);
    d->widgets.append(widget);
    connect(this, &ConfigItem::enabledChanged, wrapper, &InputWidgetWrapper::setEnabled);
    connect(widget, &QWidget::destroyed,
        [=]() {
            d->widgets.removeOne(widget);
        }
    );
    doInitWidget(widget, wrapper);
    return wrapper;
}

QString ConfigItem::toString() const
{
    QString msg = QString("[group=%1, name=%2, title=%3, description=%4, value=%5]")
        .arg(group()->name())
        .arg(name())
        .arg(title())
        .arg(description())
        .arg(value().toString());
    return msg;
}

QJsonObject ConfigItem::toJson() const
{
    Q_D(const ConfigItem);
    if (d->toJsonHook)
    {
        return doToJsonHook();
    }
    else
    {
        QJsonObject item;
        item["value"] = QJsonValue::fromVariant(value());
        item["defaultValue"] = QJsonValue::fromVariant(defaultValue());
        return item;
    }
}

void ConfigItem::fromJson(const QJsonObject& jsonObject)
{
    Q_D(ConfigItem);
    QVariant oldValue = value();
    if (d->fromJsonHook)
    {
        doFromJsonHook(jsonObject);
        d->dirtyValue = d->value;
        //d->modified = false;
    }
    else
    {
        if (jsonObject.contains("value"))
        {
            d->value = d->dirtyValue = jsonObject["value"].toVariant();
            //d->modified = false;
        }

        if (jsonObject.contains("defaultValue"))
        {
            setDefaultValue(jsonObject["defaultValue"].toVariant());
        }
    }
    d->modifiedBy = MB_ConfigFile;
    bool changed = oldValue != value();
    if (changed)
    {
        emit valueChanged(value(), MB_ConfigFile);
    }
}

DataType ConfigItem::dataType() const
{
    Q_D(const ConfigItem);
    return d->dataType;
}

void ConfigItem::setInputWidgetType(InputWidgetType widgetType)
{
    Q_D(ConfigItem);
    d->inputWidgetType = widgetType;
}

InputWidgetType ConfigItem::inputWidgetType() const
{
    Q_D(const ConfigItem);
    return d->inputWidgetType;
}

QMap<QString, QVariant>& ConfigItem::inputWidgetProperties()
{
    Q_D(ConfigItem);
    return d->inputWidgetProperties;
}

void ConfigItem::setInputWidgetProperty(const QString& key, const QVariant& value)
{
    Q_D(ConfigItem);
    if (d->inputWidgetProperties.contains(key))
    {
        d->inputWidgetProperties[key] = value;
    }
    else
    {
        d->inputWidgetProperties.insert(key, value);
    }
}

ConfigItem::WidgetInitializeHook ConfigItem::widgetInitializeHook()
{
    Q_D(ConfigItem);
    return d->widgetInitializeHook;
}

void ConfigItem::setWidgetInitializeHook(ConfigItem::WidgetInitializeHook fn)
{
    Q_D(ConfigItem);
    d->widgetInitializeHook = fn;
}

void ConfigItem::doInitWidget(QWidget* widget, InputWidgetWrapper* wrapper)
{
    Q_D(ConfigItem);
    if (widget && d->widgetInitializeHook)
    {
        widget->blockSignals(true);
        d->widgetInitializeHook(widget, this, wrapper);
        widget->blockSignals(false);
    }
}

ConfigItem::ValueHook ConfigItem::valueToWidgetHook()
{
    Q_D(ConfigItem);
    return d->valueToWidgetHook;
}

void ConfigItem::setValueToWidgetHook(ValueHook fn)
{
    Q_D(ConfigItem);
    d->valueToWidgetHook = fn;
}

QVariant ConfigItem::doValueToWidgetHook(const QVariant& value) const
{
    Q_D(const ConfigItem);
    if (d->valueToWidgetHook)
    {
        return d->valueToWidgetHook(value);
    }
    return value;
}

ConfigItem::ValueHook ConfigItem::valueFromWidgetHook()
{
    Q_D(ConfigItem);
    return d->valueFromWidgetHook;
}

void ConfigItem::setValueFromWidgetHook(ValueHook fn)
{
    Q_D(ConfigItem);
    d->valueFromWidgetHook = fn;
}

QVariant ConfigItem::doValueFromWidgetHook(const QVariant& value)
{
    Q_D(ConfigItem);
    if (d->valueFromWidgetHook)
    {
        return d->valueFromWidgetHook(value);
    }
    return value;
}

ConfigItem::CreateWidgetHook ConfigItem::createWidgetHook()
{
    Q_D(ConfigItem);
    return d->createWidgetHook;
}

void ConfigItem::setCreateWidgetHook(CreateWidgetHook fn)
{
    Q_D(ConfigItem);
    d->createWidgetHook = fn;
}

QWidget* ConfigItem::doCreateWidgetHook()
{
    Q_D(ConfigItem);
    if (d->createWidgetHook)
    {
        return d->createWidgetHook(this);
    }
    return nullptr;
}

ConfigItem::DestroyHook ConfigItem::destroyHook()
{
    Q_D(ConfigItem);
    return d->destroyHook;
}

void ConfigItem::setDestroyHook(DestroyHook fn)
{
    Q_D(ConfigItem);
    d->destroyHook = fn;
}

void ConfigItem::doDestroyHook()
{
    Q_D(ConfigItem);
    if (d->destroyHook)
    {
        d->destroyHook(this);
    }
}

ConfigItem::ToJsonHook ConfigItem::toJsonHook()
{
    Q_D(ConfigItem);
    return d->toJsonHook;
}

void ConfigItem::setToJsonHook(ToJsonHook fn)
{
    Q_D(ConfigItem);
    d->toJsonHook = fn;
}

QJsonObject ConfigItem::doToJsonHook() const
{
    Q_D(const ConfigItem);
    if (d->toJsonHook)
    {
        return d->toJsonHook(this);
    }
    return QJsonObject();
}

ConfigItem::FromJsonHook ConfigItem::fromJsonHook()
{
    Q_D(ConfigItem);
    return d->fromJsonHook;
}

void ConfigItem::setFromJsonHook(ConfigItem::FromJsonHook fn)
{
    Q_D(ConfigItem);
    d->fromJsonHook = fn;
}

void ConfigItem::doFromJsonHook(const QJsonObject& json)
{
    Q_D(ConfigItem);
    if (d->fromJsonHook)
    {
        d->fromJsonHook(d->value, d->defaultValue, json, this);
    }
}

ConfigItem::ResetHook ConfigItem::resetHook()
{
    Q_D(ConfigItem);
    return d->resetHook;
}

void ConfigItem::setResetHook(ResetHook fn)
{
    Q_D(ConfigItem);
    d->resetHook = fn;
}

void ConfigItem::doResetHook()
{
    Q_D(ConfigItem);
    if (d->resetHook)
    {
        d->resetHook(this);
    }
}

ConfigItem::RestoreHook ConfigItem::restoreHook()
{
    Q_D(ConfigItem);
    return d->restoreHook;
}

void ConfigItem::setRestoreHook(RestoreHook fn)
{
    Q_D(ConfigItem);
    d->restoreHook = fn;
}

void ConfigItem::doRestoreHook()
{
    Q_D(ConfigItem);
    if (d->restoreHook)
    {
        d->restoreHook(this);
    }
}

ConfigItem::UpdateWidgetValueHook ConfigItem::updateWidgetValueHook()
{
    Q_D(ConfigItem);
    return d->updateWidgetValueHook;
}

void ConfigItem::setUpdateWidgetValueHook(UpdateWidgetValueHook fn)
{
    Q_D(ConfigItem);
    d->updateWidgetValueHook = fn;
}

bool ConfigItem::doUpdateWidgetValueHook(QWidget* widget, const QVariant& value)
{
    Q_D(ConfigItem);
    if (d->updateWidgetValueHook)
    {
        d->updateWidgetValueHook(widget, value);
        return true;
    }
    return false;
}

const QList<QWidget*>& ConfigItem::boundedWidgets() const
{
    Q_D(const ConfigItem);
    return d->widgets;
}

ModifiedBy ConfigItem::modifiedBy() const
{
    Q_D(const ConfigItem);
    return d->modifiedBy;
}

void ConfigItem::setValue(const QVariant& value, ModifiedBy modifiedBy)
{
    Q_D(ConfigItem);
    if (!value.isValid() || value.isNull())
        return;

    d->modifiedBy = modifiedBy;
    bool changed = value != d->value;

    switch(modifiedBy)
    {
    case MB_Manual:
        d->dirtyValue = value;
        break;
    case MB_Widget:
        d->dirtyValue = value;
        break;
    case MB_ConfigFile:
        changed = false;
        d->value = d->dirtyValue = value;
        break;
    case MB_Register:
        changed = false;
        d->value = d->dirtyValue = value;
        break;
    case MB_RegisterConfirmed:
        changed = false;
        d->value = d->dirtyValue = value;
        break;
    }

    // 如果当前的保存策略是SS_DIRECTLY，那么无论前述值如何处理，
    // dirtyValue和value都会一致。
    if (d->storeStrategy == SS_DIRECTLY)
    {
        d->value = d->dirtyValue = value;
    }
    
    emit modifiedChanged(changed);
    
    if (changed)
    {
        emit valueChanged(value, modifiedBy);
    }
}

void ConfigItem::reset()
{
    Q_D(ConfigItem);
    if (isModified())
    {
        if (d->resetHook)
        {
            doResetHook();
        }
        else
        {
            d->dirtyValue = d->value;
            //d->modified = false;
        }

        emit modifiedChanged(false);
    }
}

void ConfigItem::restore()
{
    Q_D(ConfigItem);
    if (d->restoreHook)
    {
        doRestoreHook();
    }
    else
    {
        setValue(d->defaultValue, MB_Widget);
    }
}

void ConfigItem::restoreSystem()
{
    Q_D(ConfigItem);
    setValue(d->systemDefaultValue, MB_Widget);
}

void ConfigItem::confirm()
{
    Q_D(ConfigItem);
    if (d->storeStrategy == SS_CONFIRMED)
        d->value = d->dirtyValue;
}

void ConfigItem::setName(const QString& name)
{
    Q_D(ConfigItem);
    d->name = name;
}

void ConfigItem::setDescription(const QString& description)
{
    Q_D(ConfigItem);
    d->description = description;
}

void ConfigItem::onRegisterLoaded(const QVariant& value)
{
    Q_D(ConfigItem);
    setValue(value, MB_Register);
}

QDebug operator<<(QDebug debug, const ConfigItem& item)
{
    QDebugStateSaver saver(debug);
    debug << item.toString();
    return debug;
}
