#ifndef CONFIGITEM_H
#define CONFIGITEM_H

#include <QDebug>
#include <QObject>
#include "common/common.h"

class Config;
class ConfigItemGroup;
class InputWidgetWrapper;
class LaserRegister;

class ConfigItemPrivate;
class ConfigItem: public QObject
{
    Q_OBJECT
public:
    explicit ConfigItem(QObject* parent = nullptr);
    ConfigItem(const QString& name
        , ConfigItemGroup* group
        , const QString& title
        , const QString& description
        , const QVariant& value
        , bool advanced = false
        , bool visible = true
        , StoreStrategy storeType = StoreStrategy::SS_CONFIRMED
        , QObject* parent = nullptr
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

    StoreStrategy storeType() const;
    void setStoreType(StoreStrategy type);

    QVariant value() const;
    void setValue(const QVariant& value);

    QVariant defaultValue() const;
    void setDefaultValue(const QVariant& value);

    QVariant systemDefaultValue() const;
    void setSystemDefaultValue(const QVariant& value);

    QVariant lastValue() const;
    void setLastValue(const QVariant& value);

    bool isDirty() const;
    void setDirty(bool dirty);

    bool isModified() const;

    InputWidgetWrapper* bindWidget(QWidget* widget);
    LaserRegister* bindRegister(LaserRegister* reg);
    void unbindWidget(InputWidgetWrapper* widgetWrapper);
    void unbindRegister(LaserRegister* reg);

    QString toString() const;

public slots:

protected:
    void setModified();
    void setName(const QString& name);
    void setDescription(const QString& description);

    void setValueDirectly(const QVariant& value);
    void setValueConfirmed(const QVariant& value);
    void setValueLazy(const QVariant& value);

    InputWidgetWrapper* findWidget(QWidget* widget) const;

signals:
    void dirtyChanged(bool value);
    void visibleChanged(bool value);
    void valueChanged(const QVariant& value);
    void defaultValueChanged(const QVariant& value);
    void modified();

private:
    QScopedPointer<ConfigItemPrivate> m_ptr;

    Q_DECLARE_PRIVATE_D(m_ptr, ConfigItem)
    Q_DISABLE_COPY(ConfigItem)

    friend class Config;
};

QDebug operator<<(QDebug debug, const ConfigItem & item);

#endif // CONFIGITEM_H