#ifndef PROGRESSITEMMANAGER_H
#define PROGRESSITEMMANAGER_H

#include <QObject>
#include <QSet>
#include <QAbstractItemModel>
#include "task/ProgressItem.h"

class ProgressModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit ProgressModel(QObject* parent = nullptr);
    ~ProgressModel();

    ProgressItem* createItem(const QString& title, ProgressItem::ProgressType type, ProgressItem* parent = nullptr);
    ProgressItem* createSimpleItem(const QString& title, ProgressItem* parent);
    ProgressItem* createComplexItem(const QString& title, ProgressItem* parent);
    void updateItem(ProgressItem* item);

    qreal progress() const;

    void clear();

    // Inherited via QAbstractItemModel
    virtual Q_INVOKABLE QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    virtual Q_INVOKABLE QModelIndex parent(const QModelIndex& child) const override;
    virtual Q_INVOKABLE int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual Q_INVOKABLE int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual Q_INVOKABLE QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

protected:
    ProgressItem* getItem(const QModelIndex& index) const;

protected slots:

signals:
    void progressUpdated(qreal value);

private:
    QList<ProgressItem*> m_items;

    Q_DISABLE_COPY(ProgressModel)

};

#endif // PROGRESSITEMMANAGER_H