#ifndef LASERREGISTER_H
#define LASERREGISTER_H

#include <QObject>

class LaserRegisterPrivate;
class LaserRegister : public QObject
{
    Q_OBJECT
public:
    explicit LaserRegister(QObject* parent = nullptr);
    explicit LaserRegister(int addr, const QVariant& defaultValue, QObject* parent = nullptr);

private:
    QScopedPointer<LaserRegisterPrivate> m_ptr;
    Q_DECLARE_PRIVATE_D(m_ptr, LaserRegister);
    Q_DISABLE_COPY(LaserRegister);
};

#endif // LASERREGISTER_H