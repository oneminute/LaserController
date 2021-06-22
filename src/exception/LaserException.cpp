#include "LaserException.h"

#include "common/common.h"

void LaserException::raise() const
{
    throw* this;
}

LaserException* LaserException::clone() const
{
    return new LaserException(m_errorCode, m_errorMessage);
}

QString LaserException::toString() const
{
    return QString("LaserException(code=%1, message=%2)").arg(m_errorCode).arg(m_errorMessage);
}
