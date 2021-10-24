#ifndef PROGRESSITEMMANAGER_H
#define PROGRESSITEMMANAGER_H

#include <QObject>
#include <QSet>

class ProgressItem;

class ProgressItemManager : public QObject
{
    Q_OBJECT
public:
    explicit ProgressItemManager(QObject* parent = nullptr);
    ~ProgressItemManager();

private:
    QSet<ProgressItem*> m_items;

    Q_DISABLE_COPY(ProgressItemManager)
};

#endif // PROGRESSITEMMANAGER_H