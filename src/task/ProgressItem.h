#ifndef PROGRESSITEM_H
#define PROGRESSITEM_H

#include <QDateTime>
#include <QElapsedTimer>
#include <QObject>
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

    explicit ProgressItem(ProgressType processType, const QString& title, QObject* parent = nullptr);
    ~ProgressItem();

    qreal progress() const;
    void setProgress(qreal process);
    void addProgress(qreal delta);

    QString title() const { return m_title; }
    void setTitle(const QString& title) { m_title = title; }

    QString message() const { return m_message; }
    void setMessage(const QString& message) { m_message = message; }

    bool isFinished() const;

    qreal durationSecs() const;
    qreal durationMSecs() const;
    qint64 durationNSecs() const;

    void addChildItem(ProgressItem* item);
    QSet<ProgressItem*> childItems() const;

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
    void processUpdated(qreal progress);

private:
    ProgressState m_state;
    ProgressType m_type;
    qreal m_progress;
    QString m_title;
    QString m_message;
    QElapsedTimer m_timer;

    qint64 m_durationNSecs;
    QSet<ProgressItem*> m_childItems;
    QMap<ProgressItem*, qreal> m_weights;
    qreal m_sumWeights;

    Q_DISABLE_COPY(ProgressItem)
};

#endif // PROGRESSITEM_H