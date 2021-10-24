#include "ProgressItem.h"

#include <QtMath>

#include "common/common.h"

ProgressItem::ProgressItem(ProgressType processType, const QString& title, QObject* parent)
    : QObject(parent)
    , m_state(PS_Idle)
    , m_type(processType)
    , m_progress(0.0)
    , m_title("")
    , m_message("")
{

}

ProgressItem::~ProgressItem()
{
}

qreal ProgressItem::progress() const 
{
    switch (m_type)
    {
    case PT_Simple:
    {
        return m_progress; 
    }
    case PT_Complex:
    {
        qreal finalProcess = 0.0;
        for (ProgressItem* item : m_childItems)
        {
            qreal weight = m_weights[item];
            finalProcess += weight * item->progress();
        }
        return finalProcess;
    }
    }
}

void ProgressItem::setProgress(qreal process)
{
    if (m_type == PT_Simple)
        m_progress = qBound(0.0, process, 1.0); 
}

void ProgressItem::addProgress(qreal delta)
{
    if (m_type == PT_Simple)
        m_progress = qBound(0.0, m_progress + delta, 1.0);
}

bool ProgressItem::isFinished() const
{
    return progress() >= 1.0;
}

qreal ProgressItem::durationSecs() const
{
    return durationNSecs() / 1000000.0;
}

qreal ProgressItem::durationMSecs() const
{
    return durationNSecs() / 1000.0;
}

qint64 ProgressItem::durationNSecs() const
{
    if (isFinished())
    {
        return m_durationNSecs;
    }
    else
    {
        return m_timer.nsecsElapsed();
    }
}

void ProgressItem::addChildItem(ProgressItem* item)
{
    m_childItems.insert(item);
}

QSet<ProgressItem*> ProgressItem::childItems() const
{
    return m_childItems;
}

void ProgressItem::setWeight(ProgressItem* item, qreal weight)
{
    m_weights[item] = weight;
}

void ProgressItem::startTimer()
{
    m_timer.start();
}

void ProgressItem::stopTimer()
{
    m_durationNSecs = m_timer.nsecsElapsed();
}

bool ProgressItem::operator==(const ProgressItem& item)
{
    return progress() == item.progress();
}

bool ProgressItem::operator<(const ProgressItem& item)
{
    return progress() < item.progress();
}

bool ProgressItem::operator>(const ProgressItem& item)
{
    return progress() > item.progress();
}

void ProgressItem::debugPrint()
{
    qLogD << title() << ", " << m_durationNSecs;
}

QString ProgressItem::toString() const
{
    return QString();
}

void ProgressItem::updateWeights()
{
    m_sumWeights = 0;
    for (ProgressItem* item : m_childItems)
    {
        qreal weight = 1.0;
        if (m_weights.contains(item))
        {
            weight = m_weights[item];
        }
        else
        {
            m_weights.insert(item, 1.0);
        }

        m_sumWeights += weight;
    }
}

void ProgressItem::notify()
{
    ProgressItem* parent = qobject_cast<ProgressItem*>(this->parent());
    if (parent)
    {
        parent->notify();
    }
    emit processUpdated(progress());
}
