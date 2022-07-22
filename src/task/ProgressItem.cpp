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
    //QMutexLocker locker(&m_childMutex);
    m_maximum = value; 
    if (m_type == PT_Complex)
        updateWeights();
}

int ProgressItem::indexOfParent()
{
    //QMutexLocker locker(&m_childMutex);
    if (parent())
        return parent()->childItems().indexOf(this);
    return -1;
    //else
        //return LaserApplication::progressModel->m_items.indexOf(this);
}

qreal ProgressItem::progress() const
{
    if (m_isFinished)
        return 1.0;

    qreal progress = 0;
    switch (m_type)
    {
    case PT_Simple:
    {
        progress = qBound(0.0, (m_progress - m_minimum) / (m_maximum - m_minimum), 1.0); 
        break;
    }
    case PT_Complex:
    {
        //QMutexLocker locker(const_cast<QMutex*>(&m_childMutex));
        qreal finalProgress = 0.0;
        
        //qreal maximum = qMax(m_maximum, static_cast<qreal>(m_childItems.count()));
        int maximum = m_weights.size();
        for (int i = 0; i < maximum; i++)
        {
            ProgressItem* item = nullptr;
            qreal weight = m_weights[i] / m_totalWeight;
            qreal subProgress = 0;
            if (i < m_childItems.size())
            {
                item = m_childItems.at(i);
                subProgress = item->progress() * weight;
            }
            finalProgress += subProgress;
        }
        progress = qBound(0.0, finalProgress, 1.0); 
        //qLogD << finalProgress << " " << progress;
        break;
    }
    }
    return progress;
}

void ProgressItem::setProgress(qreal progress)
{
    //QMutexLocker locker(&m_childMutex);
    if (m_type == PT_Simple)
        m_progress = qBound(m_minimum, progress, m_maximum); 
    notify();
}

void ProgressItem::increaseProgress(qreal delta)
{
    //QMutexLocker locker(&m_childMutex);
    if (m_type == PT_Simple)
        m_progress = qBound(m_minimum, m_progress + delta, m_maximum); 
    notify();
}

void ProgressItem::finish()
{
    //QMutexLocker locker(&m_childMutex);
    if (m_type == PT_Simple)
        m_progress = m_maximum;
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
    notify();
}

bool ProgressItem::isFinished() const
{
    //QMutexLocker locker(const_cast<QMutex*>(&m_childMutex));
    return progress() >= 1.0;
}

qreal ProgressItem::durationSecs() const
{
    //QMutexLocker locker(const_cast<QMutex*>(&m_childMutex));
    return durationNSecs() * 0.000001;
}

qreal ProgressItem::durationMSecs() const
{
    //QMutexLocker locker(const_cast<QMutex*>(&m_childMutex));
    return durationNSecs() * 0.000001;
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
    m_childItems.append(item);
}

QList<ProgressItem*> ProgressItem::childItems() const
{
    return m_childItems;
}

ProgressItem* ProgressItem::child(int index) const
{
    if (index < m_childItems.length() && index >= 0)
        return m_childItems.at(index);
    return nullptr;
}

int ProgressItem::childCount() const
{
    return m_childItems.count();
}

QVector<qreal> ProgressItem::weights() const
{
    return m_weights;
}

void ProgressItem::setWeights(const QVector<qreal> weights)
{
    if (m_type == PT_Simple)
        return;

    m_weights.resize(qRound(m_maximum));
    for (int i = 0; i < m_weights.size(); i++)
    {
        if (i < weights.size())
            m_weights[i] = weights[i];
        else
            m_weights[i] = 1;
    }
    updateWeights();
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

void ProgressItem::clear()
{
    for (ProgressItem* item : m_childItems)
    {
        item->clear();
        delete item;
    }
    m_childItems.clear();
    m_isFinished = false;
}

void ProgressItem::reset()
{
    /*if (!m_isFinished)
    {
        qLogW << "previouse task is running.";
    }*/
    clear();
    m_progress = 0;
    emit progressUpdated(0);
}

void ProgressItem::updateWeights()
{
    int maximum = qRound(m_maximum);
    if (m_weights.size() < maximum)
    {
        int i = m_weights.size();
        m_weights.resize(maximum);
        for (; i < m_weights.size(); i++)
        {
            m_weights[i] = 1.0;
        }
    }
    else if (m_weights.size() > maximum)
    {
        m_weights.resize(maximum);
    }
    m_totalWeight = 0;
    for (qreal weight : m_weights)
    {
        m_totalWeight += weight;
    }
}

void ProgressItem::notify()
{
    if (m_parent)
    {
        m_parent->notify();
    }
    else
    {
        qreal finalProgress = progress();
        emit progressUpdated(finalProgress);
    }
}
