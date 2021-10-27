#include "ProgressModel.h"

#include <QStack>
#include "LaserApplication.h"

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
    updateItem(item);
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

void ProgressModel::updateItem(ProgressItem* item)
{
    ProgressItem* parent = item->parent();
    int row = 0;
    if (parent)
    {
        row = parent->childItems().indexOf(item);
    }
    else
    {
        row = m_items.indexOf(item);
    }

    emit dataChanged(createIndex(row, 0), createIndex(row, 1));
    emit progressUpdated(progress());
}

qreal ProgressModel::progress() const
{
    qreal value = 0;
    for (ProgressItem* child : m_items)
    {
        value += child->progress();
    }
    return value;
}

void ProgressModel::clear()
{
    QStack<ProgressItem*> stack;
    for (ProgressItem* item : m_items)
    {
        stack.push(item);
    }
    while (!stack.isEmpty())
    {
        ProgressItem* item = stack.pop();

        if (item->hasChildren())
        {
            for (ProgressItem* child : item->childItems())
            {
                stack.push(child);
            }
        }
        delete item;
    }
    m_items.clear();
    emit dataChanged(QModelIndex(), QModelIndex());
}

QModelIndex ProgressModel::index(int row, int column, const QModelIndex& parent) const
{
    //qLogD << "index: " << row << ", " << column;
    if (parent.isValid())
    {
        ProgressItem* parentItem = getItem(parent);
        ProgressItem* childItem = parentItem->child(row);
        //qLogD << "parent: " << parentItem->title() << ", child: " << childItem->title();
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
            //qLogD << "top item: " << item->title();
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
    if (!childItem)
        return QModelIndex();

    ProgressItem* parentItem = childItem->parent();

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
