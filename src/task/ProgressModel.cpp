#include "ProgressModel.h"

#include <QStack>

ProgressModel::ProgressModel(QObject* parent)
    : QAbstractItemModel(parent)
{

}

ProgressModel::~ProgressModel()
{
    clear();
}

ProgressItem* ProgressModel::createItem(const QString& title, ProgressItem::ProgressType type, ProgressItem* parent)
{
    ProgressItem* item = new ProgressItem(title, type, parent);
    if (!parent)
        m_items.append(item);
    return item;
}

ProgressItem* ProgressModel::createSimpleItem(const QString& title, ProgressItem* parent)
{
    return createItem(title, ProgressItem::PT_Simple, parent);
}

ProgressItem* ProgressModel::createComplexItem(const QString& title, ProgressItem* parent)
{
    return createItem(title, ProgressItem::PT_Complex, parent);
}

void ProgressModel::clear()
{
    for (ProgressItem* item : m_items)
    {
        item->deleteLater();
    }
    m_items.clear();
}

QModelIndex ProgressModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        ProgressItem* parentItem = getItem(parent);
        ProgressItem* childItem = parentItem->child(row);
        if (childItem)
            return createIndex(row, column, childItem);
        else
            return QModelIndex();
    }
    else
    {
        if (row < m_items.count())
        {
            ProgressItem* item = m_items.at(row);
            if (item)
                return createIndex(row, column, item);
            else
                return QModelIndex();
        }
        return QModelIndex();
    }
}

QModelIndex ProgressModel::parent(const QModelIndex& child) const
{
    if (!child.isValid())
        return QModelIndex();

    ProgressItem* childItem = getItem(child);
    ProgressItem* parentItem = qobject_cast<ProgressItem*>(childItem->parent());

    if (parentItem)
        return createIndex(parentItem->childCount(), 0, parentItem);
    else
        return QModelIndex();
}

int ProgressModel::rowCount(const QModelIndex& parent) const
{
    ProgressItem* item = getItem(parent);
    if (item)
    {
        return item->childItems().count();
    }
    else
    {
        return m_items.count();
    }
}

int ProgressModel::columnCount(const QModelIndex& parent) const
{
    return 2;
}

QVariant ProgressModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    ProgressItem* item = getItem(index);

    if (index.column() == 0)
        return item->title();
    else if (index.column() == 1)
        return item->progress();
}

ProgressItem* ProgressModel::getItem(const QModelIndex& index) const
{
    if (index.isValid())
    {
        ProgressItem* item = static_cast<ProgressItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return nullptr;
}
