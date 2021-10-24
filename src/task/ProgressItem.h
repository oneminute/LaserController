#ifndef PROGRESSITEM_H
#define PROGRESSITEM_H

#include <QDateTime>
#include <QElapsedTimer>
#include <QObject>
#include <QList>
#include <QSet>
#include <QMap>

class ProgressItem : public QObject
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

protected:
    explicit ProgressItem(const QString& title, ProgressType progressType = PT_Simple, QObject* parent = nullptr);
    ~ProgressItem();

public:
    ProgressType progressType() const { return m_type; }

    qreal minimum() const { return m_minimum; }
    void setMinimum(qreal value) { m_minimum = value; }

    qreal maximum() const { return m_maximum; }
    void setMaximum(qreal value) { m_maximum = value; }

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

    void setWeight(ProgressItem* item, qreal weight);

    virtual void startTimer();
    virtual void stopTimer();

    virtual bool operator==(const ProgressItem& item);
    virtual bool operator<(const ProgressItem& item);
    virtual bool operator>(const ProgressItem& item);

    void debugPrint();
    QString toString() const;

protected:
    void updateWeights();
    void notify();

signals:
    void progressUpdated(qreal progress);
    void finished();

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
    QMap<ProgressItem*, qreal> m_weights;
    qreal m_sumWeights;

    Q_DISABLE_COPY(ProgressItem)

    friend class ProgressModel;
};

#endif // PROGRESSITEM_H