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
        , dirty(false)
        , modified(false)
        , inputWidgetType(IWT_EditSlider)
        , laserRegister(nullptr)
        , widgetInitializeHook(nullptr)
        , loadDataHook(nullptr)
        , saveDataHook(nullptr)
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
    /// 是否为只读选项
    /// </summary>
    bool readOnly;

    /// <summary>
    /// 保存方式
    /// </summary>
    StoreStrategy storeType;

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
    /// 当前选项的值已修改，但未确认。该情况一般发生在绑定了寄存器的条件下，用户通过界面控件修改了值，
    /// 但是板卡尚未返回该寄存器的值。
    /// </summary>
    bool dirty;

    /// <summary>
    /// 是否已经被修改
    /// </summary>
    bool modified;

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

    //QList<InputWidgetWrapper*> widgetWrappers;
    LaserRegister* laserRegister;
    ConfigItem::WidgetInitializeHook widgetInitializeHook;
    ConfigItem::ValueHook loadDataHook;
    ConfigItem::ValueHook saveDataHook;
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
    , StoreStrategy storeType)
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
    d->storeType = storeType;

    switch (d->dataType)
        {
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
        }
}

ConfigItem::~ConfigItem()
{
    qLogD << "ConfigItem " << this << " destroied";
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

StoreStrategy ConfigItem::storeType() const
{
    Q_D(const ConfigItem);
    return d->storeType;
}

void ConfigItem::setStoreType(StoreStrategy type)
{
    Q_D(ConfigItem);
    d->storeType = type;
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

bool ConfigItem::isDirty() const
{
    return false;
}

void ConfigItem::setDirty(bool dirty)
{
    Q_D(ConfigItem);
    d->dirty = dirty;
}

bool ConfigItem::isModified() const
{
    Q_D(const ConfigItem);
    return d->modified;
}

InputWidgetWrapper* ConfigItem::createInputWidgetWrapper(QWidget* widget)
{
    Q_D(ConfigItem);
    return new InputWidgetWrapper(widget, this);
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
    QJsonObject item;
    item["value"] = QJsonValue::fromVariant(value());
    item["defaultValue"] = QJsonValue::fromVariant(defaultValue());
    return item;
}

void ConfigItem::fromJson(const QJsonObject& jsonObject)
{
    Q_D(ConfigItem);
    if (jsonObject.contains("value"))
    {
        d->value = d->dirtyValue = jsonObject["value"].toVariant();
        d->dirty = d->modified = false;
        emit valueChanged(d->value);
    }

    if (jsonObject.contains("defaultValue"))
    {
        setDefaultValue(jsonObject["defaultValue"].toVariant());
    }
}

QString ConfigItem::toRegisterString() const
{
    Q_D(const ConfigItem);
    if (hasRegister())
    {
        return d->laserRegister->toString();
    }
}

LaserRegister::RegisterPair ConfigItem::keyValuePair() const
{
    Q_D(const ConfigItem);
    if (hasRegister())
    {
        return d->laserRegister->keyValuePair();
    }
    else
    {
        LaserRegister::RegisterPair pair(-1, QVariant::Invalid);
        return pair;
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

ConfigItem::ValueHook ConfigItem::loadDataHook()
{
    Q_D(ConfigItem);
    return d->loadDataHook;
}

void ConfigItem::setLoadDataHook(ValueHook fn)
{
    Q_D(ConfigItem);
    d->loadDataHook = fn;
}

QVariant ConfigItem::doLoadDataHook(const QVariant& value) const
{
    Q_D(const ConfigItem);
    if (d->loadDataHook)
    {
        return d->loadDataHook(value);
    }
    return value;
}

ConfigItem::ValueHook ConfigItem::saveDataHook()
{
    Q_D(ConfigItem);
    return d->saveDataHook;
}

void ConfigItem::setSaveDataHook(ValueHook fn)
{
    Q_D(ConfigItem);
    d->saveDataHook = fn;
}

QVariant ConfigItem::doSaveDataHook(const QVariant& value)
{
    Q_D(ConfigItem);
    if (d->saveDataHook)
    {
        return d->saveDataHook(value);
    }
    return value;
}

void ConfigItem::initWidget(QWidget* widget)
{
    Q_D(ConfigItem);
    if (widget && d->widgetInitializeHook)
    {
        d->widgetInitializeHook(widget, this);
    }
}

LaserRegister* ConfigItem::laserRegister() const
{
    Q_D(const ConfigItem);
    return d->laserRegister;
}

void ConfigItem::bindLaserRegister(int addr, bool isSystem, StoreStrategy storeStrategy)
{
    Q_D(ConfigItem);
    d->laserRegister = new LaserRegister(addr, title(), dataType(), description(), isSystem, readOnly(), storeStrategy, this);
    QVariant regValue = d->laserRegister->value();
    regValue = (regValue.isValid() && !regValue.isNull()) ? regValue : defaultValue();
    d->value = d->dirtyValue = regValue; //doLoadDataHook(regValue);
    d->dirty = d->modified = false;
    connect(d->laserRegister, &LaserRegister::valueChanged, this, &ConfigItem::loadValue);
}

bool ConfigItem::hasRegister() const
{
    Q_D(const ConfigItem);
    return d->laserRegister != nullptr;
}

void ConfigItem::setValue(const QVariant& value)
{
    if (!value.isValid() || value.isNull())
        return;

    if (modifyValue(value))
    {
        emit valueChanged(value);
        emit widgetValueChanged(value);
    }
}

void ConfigItem::fromWidget(const QVariant& value)
{
    if (!value.isValid() || value.isNull())
        return;

    if (modifyValue(value))
    {
        emit valueChanged(value);
    }
}

void ConfigItem::loadValue(const QVariant& value)
{
    Q_D(ConfigItem);
    if (value != d->value)
    {
        d->value = d->dirtyValue = value;
        emit valueChanged(d->value);
    }
}

void ConfigItem::reset()
{
    Q_D(ConfigItem);
    if (d->modified)
    {
        d->dirtyValue = d->value;
        d->dirty = false;
        d->modified = false;

        emit modifiedChanged(false);
    }
}

void ConfigItem::doModify()
{
    Q_D(ConfigItem);
    if (d->modified)
    {
        d->value = d->dirtyValue;
        d->modified = false;
        d->dirty = false;

        emit modifiedChanged(false);
    }
}

void ConfigItem::restore()
{
    Q_D(ConfigItem);
    setValue(d->defaultValue);
}

void ConfigItem::restoreSystem()
{
    Q_D(ConfigItem);
    setValue(d->systemDefaultValue);
}

bool ConfigItem::modifyValue(const QVariant& value)
{
    Q_D(ConfigItem);
    bool changed = value != d->value;
    d->dirtyValue = value;
    if (changed)
    {
        d->modified = true;
        d->dirty = true;
        emit modifiedChanged(true);
    }
    else
    {
        emit modifiedChanged(false);
    }
    return changed;
}

void ConfigItem::setModified()
{
    Q_D(ConfigItem);
    d->modified = true;
    emit modifiedChanged(d->modified);
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

void ConfigItem::setValueDirectly(const QVariant& value)
{
    Q_D(ConfigItem);
    d->lastValue = d->value;
    d->dirtyValue = d->value = value;
    setModified();
}

void ConfigItem::setValueConfirmed(const QVariant& value)
{
    Q_D(ConfigItem);
    d->dirtyValue = value;
    d->dirty = true;
}

void ConfigItem::setValueLazy(const QVariant& value)
{
    Q_D(ConfigItem);
    d->dirtyValue = value;
    d->dirty = true;
}

QDebug operator<<(QDebug debug, const ConfigItem& item)
{
    QDebugStateSaver saver(debug);
    debug << item.toString();
    return debug;
}
