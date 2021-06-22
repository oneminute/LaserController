#ifndef LASEREVENT_H
#define LASEREVENT_H

#include <QObject>
#include <QEvent>

class LaserEvent : public QEvent
{
public:
    explicit LaserEvent(int eventId);
    ~LaserEvent();

    int eventId() const;

protected:
    int m_eventId;
};

class ComPortEvent : public LaserEvent
{
public:
    explicit ComPortEvent(QString portName, int eventId);
    ~ComPortEvent();

protected:
    QString m_portName;
};

#endif // LASEREVENT_H