#ifndef LASERREGISTER_H
#define LASERREGISTER_H

#include <QObject>
#include "common/common.h"

class ConfigItem;
class LaserDriver;
class LaserRegisterPrivate;
class LaserRegister : public QObject
{
    Q_OBJECT
public:
    typedef QMap<int, QVariant> RegistersMap;
    typedef QPair<int, QVariant> RegisterPair;

    explicit LaserRegister(int addr, ConfigItem* configItem, 
        bool isSystem = true, 
        bool readOnly = false, 
        bool writeOnly = false,
        QObject* parent = nullptr);
    ~LaserRegister();

    int address() const;
    QString name() const;
    QString description() const;
    bool readOnly() const;
    bool writeOnly() const;

    DataType dataType() const;

    StoreStrategy storeStrategy() const;

    ConfigItem* configItem() const;

    QVariant value() const;

    bool read();
    bool write(const QVariant& value);
    void parse(const QString& raw);

    QString toString() const;

protected slots:
    void setValue(const QVariant& value);

signals:
    void readyRead(const QVariant& value);
    void readyWritten();
    void valueLoaded(const QVariant& value);

private:
    QScopedPointer<LaserRegisterPrivate> d_ptr;
    Q_DECLARE_PRIVATE(LaserRegister)
    Q_DISABLE_COPY(LaserRegister)
};

#endif // LASERREGISTER_H