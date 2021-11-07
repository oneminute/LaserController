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
    , m_maximum(1.0)
    , m_progress(0.0)
    , m_title(title)
    , m_message("")
    , m_parent(parent)
    , m_durationNSecs(0)
    , m_isFinished(false)
{
    if (parent && parent->progressType() == PT_Complex)
    {
        parent->addChildItem(this);
    }
    if (progressType == PT_Complex)
    {
        m_maximum = 1.0;
    }
    startTimer();
}

ProgressItem::~ProgressItem()
{
}

void ProgressItem::setMaximum(qreal value) 
{
    m_maximum = value; 
    /*if (this->progressType() == PT_Complex)
    {
        updateWeights();
    }*/
}

int ProgressItem::indexOfParent()
{
    QMutexLocker locker(&m_childMutex);
    if (parent())
        return parent()->childItems().indexOf(this);
    else
        return LaserApplication::progressModel->m_items.indexOf(this);
}

qreal ProgressItem::progress() const
{
    if (m_isFinished)
        return 1.0;

    switch (m_type)
    {
    case PT_Simple:
    {
        return qBound(0.0, (m_progress - m_minimum) / (m_maximum - m_minimum), 1.0); 
    }
    case PT_Complex:
    {
        //QMutexLocker locker(const_cast<QMutex*>(&m_childMutex));
        qreal finalProgress = 0.0;
        
        qreal maximum = qMax(m_maximum, static_cast<qreal>(m_childItems.count()));
        for (ProgressItem* item : m_childItems)
        {
            //qreal weight = m_weights[item];
            finalProgress += /*weight * */item->progress() / (maximum - m_minimum);
        }
        return qBound(0.0, finalProgress, 1.0); 
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
    else if (m_type == PT_Complex)
    {
        for (ProgressItem* item : childItems())
        {
            item->finish();
        }
    }
    m_durationNSecs = m_timer.nsecsElapsed();
    stopTimer();
    m_isFinished = true;
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
    return durationNSecs() / 1000000.0;
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
    QMutexLocker locker(&m_childMutex);
    item->setParent(this);
    m_childItems.append(item);
}

QList<ProgressItem*> ProgressItem::childItems() const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_childMutex));
    return m_childItems;
}

ProgressItem* ProgressItem::child(int index) const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_childMutex));
    if (index < m_childItems.length() && index >= 0)
        return m_childItems.at(index);
    return nullptr;
}

int ProgressItem::childCount() const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_childMutex));
    return m_childItems.count();
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

//void ProgressItem::updateWeights()
//{
//    m_sumWeights = 0;
//    for (ProgressItem* item : m_childItems)
//    {
//        qreal weight = 1.0;
//        if (m_weights.contains(item))
//        {
//            weight = m_weights[item];
//        }
//        else
//        {
//            m_weights.insert(item, 1.0);
//        }
//
//        m_sumWeights += weight;
//    }
//
//    if (m_childItems.length() < m_maximum)
//    {
//        m_sumWeights += (m_maximum - m_childItems.length());
//    }
//}

void ProgressItem::notify()
{
    if (m_parent)
    {
        m_parent->notify();
    }
    LaserApplication::progressModel->updateItem(this);
}
