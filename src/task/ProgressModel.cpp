#include "ProgressModel.h"

#include <QStack>
#include "LaserApplication.h"

ProgressModel::ProgressModel(QObject* parent)
    : QAbstractItemModel(parent)
{
    qRegisterMetaType<QVector<int>>("QVector<int>");
}

ProgressModel::~ProgressModel()
{
    clear();
}

ProgressItem* ProgressModel::createItem(const QString& title, ProgressItem::ProgressType type, ProgressItem* parent)
{
    ProgressItem* item = new ProgressItem(title, type, parent);
    QModelIndex parentIndex;
    int row = 0;
    if (parent)
    {
        parentIndex = getQModelIndex(parent);
        row = parent->childCount();
    }
    else
    {
        m_items.append(item);
        row = m_items.count();
    }
    beginInsertRows(parentIndex, row, row);
    endInsertRows();
    //updateItem(item);
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
    QVector<int> roles;
    roles << Qt::DisplayRole;
    QModelIndex index1 = getQModelIndex(item);
    QModelIndex index2 = getQModelIndex(item, 2);
    emit dataChanged(index1, index2, roles);

    /*ProgressItem* parent = item->parent();
    int row = 0;
    if (parent)
    {
        row = parent->childItems().indexOf(item);
        if (parent->parent())
        {
            int parentRow = parent->parent()->childItems().indexOf(parent);
            emit dataChanged(createIndex(parentRow, 0), createIndex(parentRow, 2), roles);
        }
        else
        {
            int parentRow = m_items.indexOf(parent);
            emit dataChanged(createIndex(parentRow, 0), createIndex(parentRow, 2), roles);
        }
    }
    else
    {
        row = m_items.indexOf(item);
        emit dataChanged(createIndex(row, 0), createIndex(row, 2));
    }*/
    qreal progressValue = progress();

    //emit dataChanged(createIndex(row, 0), createIndex(row, 1));
    //emit layoutChanged();
    emit progressUpdated(progressValue);
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
    //this->removeRows(0, m_items.count());

    this->beginResetModel();
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
    this->endResetModel();
    emit dataChanged(createIndex(0, 0), createIndex(0, 2));
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
    return 3;
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
    else if (index.column() == 2)
        return tr("%1 ms").arg(item->durationMSecs(), 0, 'f', 3);
}

Q_INVOKABLE QVariant ProgressModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        if (section == 0)
            return tr("Task");
        else if (section == 1)
            return tr("Progress");
        else if (section == 2)
            return tr("Duration");
    }
    return Q_INVOKABLE QVariant();
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

QModelIndex ProgressModel::getQModelIndex(ProgressItem* item, int column)
{
    if (!item)
        return QModelIndex();

    ProgressItem* parent = item->parent();
    QModelIndex parentIndex;
    if (parent)
    {
        parentIndex = getQModelIndex(parent);
    }
    /*else
    {
        int row = m_items.indexOf(item);
        parentIndex = createIndex(row, 0, item);
    }*/
    int row = item->indexOfParent();
    QModelIndex curIndex = index(row, column, parentIndex);
    return curIndex;
}
