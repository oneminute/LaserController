#include "PointListView.h"

class PointTableModelPrivate
{
    Q_DECLARE_PUBLIC(PointTableModel)
public:
    PointTableModelPrivate(PointTableModel* ptr)
        : q_ptr(ptr)
    {}

    PointTableModel* q_ptr;
    QList<QPointF> points;
    QVariantList userDatas;
};

PointTableModel::PointTableModel(QObject* parent)
    : QAbstractTableModel(parent)
    , d_ptr(new PointTableModelPrivate(this))
{

}

PointTableModel::~PointTableModel()
{
}

QList<QPointF> PointTableModel::points() const
{
    Q_D(const PointTableModel);
    return d->points;
}

void PointTableModel::setPoints(const QList<QPointF>& points)
{
    Q_D(PointTableModel);
    d->points = points;
    d->userDatas.clear();
    for (int i = 0; i < points.count(); i++)
    {
        d->userDatas.append(QVariant());
    }
    emit dataChanged(
        QAbstractItemModel::createIndex(0, 0),
        QAbstractItemModel::createIndex(d->points.length() - 1, 1)
    );
}

QPointF PointTableModel::pointAt(int index) const
{
    Q_D(const PointTableModel);
    return d->points.at(index);
}

void PointTableModel::append(const QPointF& point)
{
    Q_D(PointTableModel);
    d->points.append(point);
    d->userDatas.append(QVariant());
    emit dataChanged(
        QAbstractItemModel::createIndex(d->points.length() - 1, 0),
        QAbstractItemModel::createIndex(d->points.length() - 1, 1)
    );
}

void PointTableModel::append(const QList<QPointF>& points)
{
    Q_D(PointTableModel);
    int oldLength = d->points.length();
    d->points.append(points);
    for (int i = 0; i < points.length(); i++)
    {
        d->userDatas.append(QVariant());
    }
    int newLength = d->points.length();
    emit dataChanged(
        QAbstractItemModel::createIndex(oldLength, 0),
        QAbstractItemModel::createIndex(newLength - 1, 1)
    );
}

void PointTableModel::removeAt(int index)
{
    Q_D(PointTableModel);
    d->points.removeAt(index);
    d->userDatas.removeAt(index);
    emit dataChanged(
        QAbstractItemModel::createIndex(index, 0),
        QAbstractItemModel::createIndex(index, 1)
    );
}

Q_INVOKABLE QModelIndex PointTableModel::index(int row, int column, const QModelIndex& parent) const
{
    return QAbstractItemModel::createIndex(row, column);
}

Q_INVOKABLE QModelIndex PointTableModel::parent(const QModelIndex& child) const
{
    return QModelIndex();
}

Q_INVOKABLE int PointTableModel::rowCount(const QModelIndex& parent) const
{
    Q_D(const PointTableModel);
    return d->points.count();
}

Q_INVOKABLE int PointTableModel::columnCount(const QModelIndex& parent) const
{
    return 2;
}

Q_INVOKABLE QVariant PointTableModel::data(const QModelIndex& index, int role) const
{
    Q_D(const PointTableModel);
    if (!index.isValid())
        return QVariant();

    QVariant value;
    switch (role)
    {
    case Qt::DisplayRole:
    {
        if (index.column() == 0)
            return d->points.at(index.row()).x();
        else if (index.column() == 1)
            return d->points.at(index.row()).y();
    }
    break;
    case Qt::UserRole:
    {
        return d->userDatas.at(index.row());
    }
        break;
    }
    return value;
}

PointTableView::PointTableView(QWidget* parent)
    : QTableView(parent)
{

}

PointTableView::~PointTableView()
{
}


