#ifndef LASEREXCEPTION_H
#define LASEREXCEPTION_H

#include <QObject>
#include <QException>
#include "laser/LaserDefines.h"

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

#define DEFINE_LASER_EXCEPTION_CODE(className, errorCode) \
    class className: public LaserException \
    { \
    public: \
        explicit className(const QString& errorMessage) \
            : LaserException(errorCode, errorMessage) \
        {} \
        void raise() const override { throw* this; } \
        className* clone() const override { return new className(m_errorMessage); } \
    }

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

DEFINE_LASER_EXCEPTION_CODE(LaserFileException, E_FileException);
DEFINE_LASER_EXCEPTION_CODE(LaserFatalException, E_SystemFatalError);
DEFINE_LASER_EXCEPTION(LaserDeviceFatalException);
DEFINE_LASER_EXCEPTION(LaserDeviceException);
DEFINE_LASER_EXCEPTION_D(LaserDeviceSecurityException, LaserDeviceException);
DEFINE_LASER_EXCEPTION_D(LaserDeviceConnectionException, LaserDeviceException);
DEFINE_LASER_EXCEPTION_D(LaserDeviceIOException, LaserDeviceException);
DEFINE_LASER_EXCEPTION_D(LaserNetworkException, LaserException);
DEFINE_LASER_EXCEPTION_D(LaserDeviceDataException, LaserDeviceException);
DEFINE_LASER_EXCEPTION_D(LaserDeviceMachiningException, LaserDeviceException);

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
