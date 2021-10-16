#ifndef POINTLISTVIEW_H
#define POINTLISTVIEW_H

#include <QTableView>
#include <QAbstractTableModel>
#include <QPointF>

class PointTableModelPrivate;
class PointTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit PointTableModel(QObject* parent = nullptr);
    ~PointTableModel();

    QList<QPointF> points() const;
    void setPoints(const QList<QPointF>& points);
    QPointF pointAt(int index) const;
    void append(const QPointF& point);
    void append(const QList<QPointF>& points);
    void removeAt(int index);

    // Inherited via QAbstractItemModel
    virtual Q_INVOKABLE QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    virtual Q_INVOKABLE QModelIndex parent(const QModelIndex& child) const override;
    virtual Q_INVOKABLE int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual Q_INVOKABLE int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual Q_INVOKABLE QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

private:
    QScopedPointer<PointTableModelPrivate> d_ptr;

    Q_DECLARE_PRIVATE(PointTableModel)
    Q_DISABLE_COPY(PointTableModel)
};

class PointTableView : public QTableView
{
    Q_OBJECT
public:
    explicit PointTableView(QWidget* parent = nullptr);
    ~PointTableView();

};

#endif // POINTLISTVIEW_H
