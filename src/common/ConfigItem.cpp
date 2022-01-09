#include "ConfigItem.h"
#include "ConfigItemGroup.h"
#include "widget/InputWidgetWrapper.h"

#include <QDateTime>
#include <QStack>

class ConfigItemPrivate
{
    Q_DECLARE_PUBLIC(ConfigItem)
public:
    ConfigItemPrivate(ConfigItem* ptr)
        : q_ptr(ptr)
        , group(nullptr)
        , advanced(false)
        , needRelaunch(false)
        , visible(true)
        , enabled(true)
        , exportable(true)
        , modified(false)
        , inputWidgetType(IWT_EditSlider)
        , widgetInitializeHook(nullptr)
        //, valueToWidgetHook(nullptr)
        , valueFromWidgetHook(nullptr)
        , createWidgetHook(nullptr)
        , destroyHook(nullptr)
        , toJsonHook(nullptr)
        , fromJsonHook(nullptr)
        , updateWidgetValueHook(nullptr)
        , retranslateHook(nullptr)
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
    /// 需要重新启动软件
    /// </summary>
    bool needRelaunch;

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
    /// 是否已修改
    /// </summary>
    bool modified;

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

    QMap<QString, QVariant> extraProperties;

    QList<QWidget*> widgets;

    QStack<QVariant> stack;

    QDateTime lastValueModifiedTime;
    QDateTime lastDirtyModifiedTime;

    ConfigItem::WidgetInitializeHook widgetInitializeHook;
    //ConfigItem::ValueHook valueToWidgetHook;
    ConfigItem::WidgetValueHook valueFromWidgetHook;
    ConfigItem::CreateWidgetHook createWidgetHook;
    ConfigItem::DestroyHook destroyHook;
    ConfigItem::ToJsonHook toJsonHook;
    ConfigItem::FromJsonHook fromJsonHook;
    ConfigItem::WidgetValueHook updateWidgetValueHook;
    ConfigItem::RetranslateHook retranslateHook;
};

ConfigItem::ConfigItem(
    const QString& name
    , ConfigItemGroup* group
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

    d->value = value;
    d->dirtyValue = value;
    d->defaultValue = value;
    d->systemDefaultValue = value;

    d->dataType = dataType;
    d->advanced = advanced;
    d->visible = visible;
    d->storeStrategy = storeStrategy;

    if (d->name == "xStepLength")
        qLogD << "debugging " << d->name;

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
    case DT_POINT:
        d->inputWidgetType = IWT_Vector2DWidget;
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

void ConfigItem::setTitle(const QString& title)
{
    Q_D(ConfigItem);
    d->title = title;
}

QString ConfigItem::description() const
{
    Q_D(const ConfigItem);
    return d->description;
}

void ConfigItem::setDescription(const QString& description)
{
    Q_D(ConfigItem);
    d->description = description;
}

void ConfigItem::setTitleAndDesc(const QString& title, const QString& desc)
{
    setTitle(title);
    setDescription(desc);
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

bool ConfigItem::needRelaunch() const
{
    Q_D(const ConfigItem);
    return d->needRelaunch;
}

bool ConfigItem::isVisible() const
{
    Q_D(const ConfigItem);
    return d->visible;
}

void ConfigItem::setVisible(bool visible)
{
    Q_D(ConfigItem);
    d->visible = visible;
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
    if (isDirty())
    {
        return d->dirtyValue;
    }
    else
    {
        return d->value;
    }
}

QVariant ConfigItem::oldValue() const
{
    Q_D(const ConfigItem);
    return d->value;
}

QVariant ConfigItem::defaultValue() const
{
    Q_D(const ConfigItem);
    return d->defaultValue;
}

void ConfigItem::setDefaultValue(const QVariant& value)
{
    Q_D(ConfigItem);
    d->defaultValue = value;
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

bool ConfigItem::isDirty() const
{
    Q_D(const ConfigItem);
    return d->dirtyValue != d->value;
}

bool ConfigItem::isModified() const
{
    Q_D(const ConfigItem);
    return d->modified;
}

InputWidgetWrapper* ConfigItem::bindWidget(QWidget* widget, StoreStrategy ss)
{
    Q_D(ConfigItem);
    for (QMap<QString, QVariant>::ConstIterator i = d->inputWidgetProperties.constBegin(); i != d->inputWidgetProperties.constEnd(); i++)
    {
        widget->setProperty(i.key().toStdString().c_str(), i.value());
    }
    InputWidgetWrapper* wrapper = new InputWidgetWrapper(widget, this);
    wrapper->setStoreStrategy(ss);
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

void ConfigItem::setNeedRelaunch(bool needRelaunch)
{
    Q_D(ConfigItem);
    d->needRelaunch = needRelaunch;
}

void ConfigItem::clearModified()
{
    Q_D(ConfigItem);
    Q_ASSERT(d->dirtyValue == d->value);
    if (d->name == "deviceOrigin")
        qLogD << "debug deviceOrigin: " << d->dirtyValue << ", " << d->value;
    d->modified = false;
    emit modifiedChanged(false);
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
        if (this == Config::Camera::resolutionItem())
        {
            qLogD << "break point: " << value().toString();
        }
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
    }
    else
    {
        if (jsonObject.contains("value"))
        {
            d->value = d->dirtyValue = jsonObject["value"].toVariant();
        }

        if (jsonObject.contains("defaultValue"))
        {
            setDefaultValue(jsonObject["defaultValue"].toVariant());
        }
    }
    d->modified = false;
    emit valueChanged(value(), this);
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

QVariantMap& ConfigItem::extraProperties()
{
    Q_D(ConfigItem);
    return d->extraProperties;
}

void ConfigItem::setExtraProperty(const QString& key, const QVariant& value)
{
    Q_D(ConfigItem);
    d->extraProperties[key] = value;
}

QVariant& ConfigItem::extraProperty(const QString& key)
{
    Q_D(ConfigItem);
    return d->extraProperties[key];
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

//ConfigItem::ValueHook ConfigItem::valueToWidgetHook()
//{
//    Q_D(ConfigItem);
//    return d->valueToWidgetHook;
//}
//
//void ConfigItem::setValueToWidgetHook(ValueHook fn)
//{
//    Q_D(ConfigItem);
//    d->valueToWidgetHook = fn;
//}
//
//QVariant ConfigItem::doValueToWidgetHook(const QVariant& value) const
//{
//    Q_D(const ConfigItem);
//    if (d->valueToWidgetHook)
//    {
//        return d->valueToWidgetHook(value);
//    }
//    return value;
//}

ConfigItem::WidgetValueHook ConfigItem::valueFromWidgetHook()
{
    Q_D(ConfigItem);
    return d->valueFromWidgetHook;
}

void ConfigItem::setValueFromWidgetHook(WidgetValueHook fn)
{
    Q_D(ConfigItem);
    d->valueFromWidgetHook = fn;
}

QVariant ConfigItem::doValueFromWidgetHook(QWidget* widget, const QVariant& value)
{
    Q_D(ConfigItem);
    if (d->valueFromWidgetHook)
    {
        return d->valueFromWidgetHook(widget, value);
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

ConfigItem::WidgetValueHook ConfigItem::updateWidgetValueHook()
{
    Q_D(ConfigItem);
    return d->updateWidgetValueHook;
}

void ConfigItem::setUpdateWidgetValueHook(WidgetValueHook fn)
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

ConfigItem::RetranslateHook ConfigItem::retranslateHook()
{
    Q_D(ConfigItem);
    return d->retranslateHook;
}

void ConfigItem::setRetranslateHook(RetranslateHook fn)
{
    Q_D(ConfigItem);
    d->retranslateHook = fn;
}

void ConfigItem::doRetranslateHook(QWidget* widget)
{
    Q_D(ConfigItem);
    if (d->retranslateHook)
    {
        d->retranslateHook(widget, this);
    }
}

const QList<QWidget*>& ConfigItem::boundedWidgets() const
{
    Q_D(const ConfigItem);
    return d->widgets;
}

void ConfigItem::setValue(const QVariant& value, StoreStrategy strategy_, void* senderPtr)
{
    Q_D(ConfigItem);
    if (!value.isValid())
        return;

    bool dirtyChanged = value != d->dirtyValue;
    bool valueChanged = value != d->value;

    StoreStrategy strategy = d->storeStrategy;
    if (strategy_ != SS_AS_IS)
        strategy = strategy_;

    switch (strategy)
    {
    break;
    case SS_DIRECTLY:
        d->dirtyValue = d->value = value;
        if (dirtyChanged)
            emit dirtyValueChanged(value, senderPtr);
        if (valueChanged)
        {
            emit this->valueChanged(value, senderPtr);
            emit modifiedChanged(true);
        }
        group()->save(true, true, true);
        break;
    case SS_REGISTER:
        d->dirtyValue = value;
        if (dirtyChanged)
            emit dirtyValueChanged(value, senderPtr);
        //if (valueChanged)
        //{
            //emit this->valueChanged(value, senderPtr);
            //emit modifiedChanged(true);
        //}
        group()->save(true, false);
        break;
    case SS_NORMAL:
        d->dirtyValue = value;
        if (dirtyChanged)
            emit dirtyValueChanged(value, senderPtr);
    case SS_LAZY:
        d->dirtyValue = value;
        if (dirtyChanged)
            emit dirtyValueChanged(value, senderPtr);
        break;
    }
}

void ConfigItem::push()
{
    Q_D(ConfigItem);
    d->stack.push(d->value);
}

void ConfigItem::pop()
{
    Q_D(ConfigItem);
    d->value = d->stack.pop();
}

void ConfigItem::reset()
{
    Q_D(ConfigItem);
    if (isDirty())
    {
        d->dirtyValue = d->value;

        emit dirtyValueChanged(d->dirtyValue, this);
        emit modifiedChanged(false);
    }
}

void ConfigItem::restoreToDefault()
{
    Q_D(ConfigItem);
    bool dirtyChanged = d->defaultValue != d->dirtyValue;
    d->dirtyValue = d->defaultValue;
    if (dirtyChanged)
        emit dirtyValueChanged(d->dirtyValue, this);
}

void ConfigItem::restoreToSystemDefault()
{
    Q_D(ConfigItem);
    bool dirtyChanged = d->systemDefaultValue != d->dirtyValue;
    d->dirtyValue = d->systemDefaultValue;
    if (dirtyChanged)
        emit dirtyValueChanged(d->dirtyValue, this);
}

void ConfigItem::apply()
{
    Q_D(ConfigItem);
    bool changed = d->value != d->dirtyValue;
    d->value = d->dirtyValue;
    if (changed)
        emit valueChanged(d->value, this);
}

void ConfigItem::applyToDefault()
{
    Q_D(ConfigItem);
    bool changed = d->value != d->dirtyValue;
    d->value = d->dirtyValue;
    d->defaultValue = d->dirtyValue;
    if (changed)
        emit valueChanged(d->value, this);
}

bool ConfigItem::confirm(const QVariant& value)
{
    Q_D(ConfigItem);
    bool success;
    if (d->dirtyValue == value)
        success = true;
    else
        success = false;

    if (success)
    {
        d->dirtyValue = d->value = value;
        clearModified();
        emit valueChanged(d->value, this);
    }
    return success;
}

void ConfigItem::loadValue(const QVariant& value)
{
    Q_D(ConfigItem);
    if (!value.isValid())
        return;

    if (d->name == "yMaxLength")
    {
        qLogD << "debug " << d->name;
    }

    bool dirtyChanged = value != d->dirtyValue;
    bool valueChanged = value != d->value;

    d->dirtyValue = d->value = value;
    clearModified();

    if (dirtyChanged)
        emit dirtyValueChanged(d->dirtyValue, this);
    if (valueChanged)
        emit this->valueChanged(d->value, this);
}

QJsonObject qSizeItemToJson(const ConfigItem* configItem)
{
    QSize value = configItem->value().toSize();
    QSize defaultValue = configItem->defaultValue().toSize();
    QJsonObject jsonObj;
    jsonObj["value"] = typeUtils::size2Json(value);
    jsonObj["defaultValue"] = typeUtils::size2Json(defaultValue);
    return jsonObj;
}

void parseQSizeItemFromJson(QVariant& value, QVariant& defaultValue, const QJsonObject& json, ConfigItem* item)
{
    if (json.contains("value"))
    {
        value = typeUtils::json2Size(json["value"]);
    }
    if (json.contains("defaultValue"))
    {
        defaultValue = typeUtils::json2Size(json["defaultValue"]);
    }
}

QJsonObject qPointItemToJson(const ConfigItem* configItem)
{
    QPoint pt = configItem->value().toPoint();
    QPoint defPt = configItem->defaultValue().toPoint();
    QJsonObject jsonObj;
    jsonObj["value"] = typeUtils::point2Json(pt);
    jsonObj["defaultValue"] = typeUtils::point2Json(defPt);
    return jsonObj;
}

void parseQPointItemFromJson(QVariant& value, QVariant& defaultValue, const QJsonObject& json, ConfigItem* item)
{
    if (json.contains("value"))
    {
        value = typeUtils::json2Point(json["value"]);
    }
    if (json.contains("defaultValue"))
    {
        defaultValue = typeUtils::json2Point(json["defaultValue"]);
    }
}

QDebug operator<<(QDebug debug, const ConfigItem& item)
{
    QDebugStateSaver saver(debug);
    debug << item.toString();
    return debug;
}
