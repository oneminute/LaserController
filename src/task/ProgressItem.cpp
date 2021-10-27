#include "ProgressItem.h"

#include <QtMath>
#include <QMutexLocker>

#include "common/common.h"
#include "LaserApplication.h"
#include "task/ProgressModel.h"

ProgressItem::ProgressItem(const QString& title, ProgressType progressType, ProgressItem* parent)
    : m_state(PS_Idle)
    , m_type(progressType)
    , m_minimum(0.0)
    , m_maximum(100.0)
    , m_progress(0.0)
    , m_title(title)
    , m_message("")
    , m_parent(parent)
{
    if (parent && parent->progressType() == PT_Complex)
    {
        parent->addChildItem(this);
    }
    startTimer();
}

ProgressItem::~ProgressItem()
{
}

qreal ProgressItem::progress() const 
{
    //QMutexLocker locker(const_cast<QMutex*>(&m_mutex));
    switch (m_type)
    {
    case PT_Simple:
    {
        return qBound(0.0, (m_progress - m_minimum) / (m_maximum - m_minimum), 1.0); 
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
    //QMutexLocker locker(const_cast<QMutex*>(&m_mutex));
    if (m_type == PT_Simple)
        m_progress = qBound(m_minimum, process, m_maximum); 
    notify();
}

void ProgressItem::increaseProgress(qreal delta)
{
    if (m_type == PT_Simple)
        setProgress(m_progress + delta);
}

void ProgressItem::finish()
{
    if (m_type == PT_Simple)
        setProgress(m_maximum);
    stopTimer();
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
    item->setParent(this);
    if (!m_childItems.contains(item))
        m_childItems.append(item);
    setWeight(item, 1.0);
}

QList<ProgressItem*> ProgressItem::childItems() const
{
    return m_childItems;
}

ProgressItem* ProgressItem::child(int index) const
{
    if (index < m_childItems.length())
        return m_childItems.at(index);
    return nullptr;
}

int ProgressItem::childCount() const
{
    return m_childItems.count();
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

ProgressItem* ProgressItem::parent() const
{
    return m_parent;
}

void ProgressItem::setParent(ProgressItem* parent)
{
    m_parent = parent;
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
    if (m_parent)
    {
        m_parent->notify();
    }
    LaserApplication::progressModel->updateItem(this);
}
