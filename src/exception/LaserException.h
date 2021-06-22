#ifndef LASEREXCEPTION_H
#define LASEREXCEPTION_H

#include "laser/LaserDefines.h"
#include <QObject>
#include <QException>

#define DEFINE_LASER_EXCEPTION_D(className, parentClassName) \
    class className: public parentClassName \
    { \
    public: \
        explicit className(int errorCode, const QString& errorMessage) \
            : parentClassName(errorCode, errorMessage) \
        {} \
        void raise() const override { throw* this; } \
        className* clone() const override { return new className(m_errorCode, m_errorMessage); } \
    }

#define DEFINE_LASER_EXCEPTION(className) \
    DEFINE_LASER_EXCEPTION_D(className, LaserException)

class LaserException : public QException
{
public:
    explicit LaserException(int errorCode, const QString& errorMessage)
        : m_errorCode(errorCode)
        , m_errorMessage(errorMessage)
    {}

    void raise() const override;

    LaserException* clone() const override;

    QString toString() const;

    int errorCode() const
    {
        return m_errorCode;
    }

    QString errorMessage() const
    {
        return m_errorMessage;
    }

protected:
    int m_errorCode;
    QString m_errorMessage;
};

DEFINE_LASER_EXCEPTION(LaserDeviceFatalException);
DEFINE_LASER_EXCEPTION(LaserDeviceException);
DEFINE_LASER_EXCEPTION_D(LaserDeviceSecurityException, LaserDeviceException);
DEFINE_LASER_EXCEPTION_D(LaserDeviceConnectionException, LaserDeviceException);
DEFINE_LASER_EXCEPTION_D(LaserDeviceIOException, LaserDeviceException);
DEFINE_LASER_EXCEPTION_D(LaserNetworkException, LaserException);
DEFINE_LASER_EXCEPTION_D(LaserDeviceDataException, LaserDeviceException);
DEFINE_LASER_EXCEPTION_D(LaserDeviceMachiningException, LaserDeviceException);

class LaserFatalException : public LaserException
{
public:
    LaserFatalException(const QString& message)
        : LaserException(E_SystemFatalError, message)
    {}
    void raise() const override { throw* this; } 
    LaserFatalException* clone() const override { return new LaserFatalException(m_errorMessage); }
};

class LaserDeviceUnknownException : public LaserException
{
public:
    LaserDeviceUnknownException(int code)
        : LaserException(code, QObject::tr("Unknown laser device error"))
    {}
    void raise() const override { throw* this; } 
    LaserDeviceUnknownException* clone() const override { return new LaserDeviceUnknownException(m_errorCode); }
};

#endif // LASEREXCEPTION_H
