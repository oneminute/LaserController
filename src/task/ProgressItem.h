#ifndef PROGRESSITEM_H
#define PROGRESSITEM_H

#include <QDateTime>
#include <QElapsedTimer>
#include <QObject>
#include <QList>
#include <QSet>
#include <QMap>
#include <QMutex>

class ProgressItem: public QObject
{
    Q_OBJECT
public:
    enum ProgressState
    {
        PS_Idle,
        PS_Running,
        PS_Finished
    };

    enum ProgressType
    {
        PT_Simple,
        PT_Complex
    };

public:
    explicit ProgressItem(const QString& title, ProgressType progressType = PT_Simple, ProgressItem* parent = nullptr);
    ~ProgressItem();

    ProgressType progressType() const { return m_type; }

    qreal minimum() const { return m_minimum; }
    void setMinimum(qreal value) { m_minimum = value; }

    qreal maximum() const { return m_maximum; }
    void setMaximum(qreal value);

    int indexOfParent();

    qreal progress() const;
    void setProgress(qreal process);
    void increaseProgress(qreal delta = 1.0);
    void finish();

    QString title() const { return m_title; }
    void setTitle(const QString& title) { m_title = title; }

    QString message() const { return m_message; }
    void setMessage(const QString& message) { m_message = message; }

    bool isFinished() const;

    qreal durationSecs() const;
    qreal durationMSecs() const;
    qint64 durationNSecs() const;

    void addChildItem(ProgressItem* item);
    QList<ProgressItem*> childItems() const;
    ProgressItem* child(int index) const;
    int childCount() const;
    bool hasChildren() const { return !m_childItems.empty(); }

    QVector<qreal> weights() const;
    void setWeights(const QVector<qreal> weights);

    virtual void startTimer();
    virtual void stopTimer();

    virtual bool operator==(const ProgressItem& item);
    virtual bool operator<(const ProgressItem& item);
    virtual bool operator>(const ProgressItem& item);

    void debugPrint();
    QString toString() const;

    ProgressItem* parent() const;
    void setParent(ProgressItem* parent);

    void clear();
    void reset();

protected:
    void updateWeights();
    void notify();

signals:
    void progressUpdated(qreal value);

private:
    ProgressState m_state;
    ProgressType m_type;
    qreal m_minimum;
    qreal m_maximum;
    qreal m_progress;
    QString m_title;
    QString m_message;
    QElapsedTimer m_timer;

    qint64 m_durationNSecs;
    QList<ProgressItem*> m_childItems;
    QVector<qreal> m_weights;
    qreal m_totalWeight;

    ProgressItem* m_parent;
    bool m_isFinished;

    QMutex m_childMutex;
    QMutex m_progressMutex;

    friend class ProgressModel;
};

#endif // PROGRESSITEM_H