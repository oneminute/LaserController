#ifndef LASERREGISTER_H
#define LASERREGISTER_H

#include <QObject>
#include "common/common.h"

class LaserRegister;
class LaserDriver;
class LaserRegisterPrivate;
class LaserRegister : public QObject
{
    Q_OBJECT
public:
    typedef QMap<int, QVariant> RegistersMap;
    typedef QPair<int, QVariant> RegisterPair;

    explicit LaserRegister(int addr, const QString& name = "", DataType dataType = DT_INT, const QString& description = "", 
        bool isSystem = true, bool readOnly = false, StoreStrategy storeStrategy = SS_CONFIRMED, QObject* parent = nullptr);
    virtual ~LaserRegister();

    int address() const;
    QString name() const;
    QString description() const;
    bool readOnly() const;

    DataType dataType() const;
    void setDataType(DataType dataType);

    StoreStrategy storeStrategy() const;
    void setStoreStrategy(StoreStrategy storeStrategy);

    bool read();
    bool write(const QVariant& value);

    QString toString() const;

protected slots:
    void setValue(const QVariant& value);
    void parse(const QString& raw);
    static void batchParse(const QString& raw, bool isSystem);

signals:
    void readyRead(const QVariant& value);
    void readyWritten();
    void valueLoaded(const QVariant& value);

private:
    static QMap<int, LaserRegister*> userRegisters;
    static QMap<int, LaserRegister*> systemRegisters;

    QScopedPointer<LaserRegisterPrivate> m_ptr;
    Q_DECLARE_PRIVATE_D(m_ptr, LaserRegister);
    Q_DISABLE_COPY(LaserRegister);

    friend class LaserDevice;
    friend class ConfigItem;
};

#endif // LASERREGISTER_H