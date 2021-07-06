#ifndef LASERREGISTER_H
#define LASERREGISTER_H

#include <QObject>

class LaserRegister;
class LaserDriver;
class LaserRegisterPrivate;
class LaserRegister : public QObject
{
    Q_OBJECT
public:

    enum StoreStrategy
    {
        SS_DIRECTLY,
        SS_LAZY
    };

    Q_ENUM(StoreStrategy)

    explicit LaserRegister(int addr, const QString& name = "", const QString& description = "", 
        bool isSystem = true, bool readOnly = false, LaserDriver* parent = nullptr);
    virtual ~LaserRegister();

    int address() const;
    QString name() const;
    QString description() const;
    bool readOnly() const;

    QVariant value() const;
    void setValue(const QVariant& value);

    bool readAsync();
    bool writeAsync();

signals:
    void readyRead(const QVariant& value);
    void readyWritten();
    void valueChanged(const QVariant& value);

private:
    QScopedPointer<LaserRegisterPrivate> m_ptr;
    Q_DECLARE_PRIVATE_D(m_ptr, LaserRegister);
    Q_DISABLE_COPY(LaserRegister);
};

#endif // LASERREGISTER_H