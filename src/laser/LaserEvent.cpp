#include "LaserEvent.h"

LaserEvent::LaserEvent(int eventId)
    : QEvent(QEvent::User)
    , m_eventId(eventId)
{
}

LaserEvent::~LaserEvent()
{
}

int LaserEvent::eventId() const
{
    return 0;
}

ComPortEvent::ComPortEvent(QString portName, int eventId)
    : LaserEvent(eventId)
    , m_portName(portName)
{
}

ComPortEvent::~ComPortEvent()
{
}
