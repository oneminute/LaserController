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
        , createWidgetFunction(nullptr)
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
    ConfigItem::CreateWidgetFn createWidgetFunction;
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

void ConfigItem::setValue(const QVariant& value)
{
    Q_D(ConfigItem);
    bool changed = value != d->value;
    d->dirtyValue = value;
    if (changed)
    {
        d->modified = true;
        d->dirty = true;
        emit valueChanged(value);
        emit modifiedChanged(true);
    }
    else
    {
        emit modifiedChanged(false);
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

LaserRegister* ConfigItem::bindRegister(LaserRegister* reg)
{
    Q_D(ConfigItem);
    d->laserRegister = reg;
    return reg;
}

void ConfigItem::unbindRegister(LaserRegister* reg)
{
    Q_D(ConfigItem);
    d->laserRegister = nullptr;
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
    if (jsonObject.contains("value"))
        setValue(jsonObject["value"].toVariant());

    if (jsonObject.contains("defaultValue"))
        setDefaultValue(jsonObject["defaultValue"].toVariant());
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

ConfigItem::CreateWidgetFn ConfigItem::createWidgetFunction()
{
    Q_D(ConfigItem);
    return d->createWidgetFunction;
}

void ConfigItem::setCreateWidgetFunction(ConfigItem::CreateWidgetFn fn)
{
    Q_D(ConfigItem);
    d->createWidgetFunction = fn;
}

void ConfigItem::initWidget(QWidget* widget)
{
    Q_D(ConfigItem);
    if (widget && d->createWidgetFunction)
    {
        d->createWidgetFunction(widget, this);
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

//InputWidgetWrapper* ConfigItem::findWidget(QWidget* widget) const
//{
//    Q_D(const ConfigItem);
//    for (InputWidgetWrapper* wrapper : d->widgetWrappers)
//    {
//        if (wrapper->widget() == widget)
//        {
//            return wrapper;
//        }
//    }
//    return nullptr;
//}

QDebug operator<<(QDebug debug, const ConfigItem& item)
{
    QDebugStateSaver saver(debug);
    debug << item.toString();
    return debug;
}
