#include "PointPairTableWidget.h"

#include <LaserApplication.h>
#include <laser/LaserDevice.h>

PointPairTableWidget::PointPairTableWidget(QWidget* parent)
    : QTableWidget(parent)
{
    QStringList columnHeaders;
    columnHeaders
        << tr("X1")
        << tr("Y1")
        << tr("X2")
        << tr("Y2");
    setColumnCount(4);
    setHorizontalHeaderLabels(columnHeaders);

    setColumnWidth(0, 50);
    setColumnWidth(1, 50);
    setColumnWidth(2, 50);
    setColumnWidth(3, 50);

    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
}

PointPairTableWidget::~PointPairTableWidget()
{
}

void PointPairTableWidget::addNewLine()
{
    insertRow(rowCount());
}

PointPairList PointPairTableWidget::pointPairs() const
{
    PointPairList list;
    for (int i = 0; i < rowCount(); i++)
    {
        QTableWidgetItem* x1Item = this->item(i, 0);
        QTableWidgetItem* y1Item = this->item(i, 1);
        QTableWidgetItem* x2Item = this->item(i, 2);
        QTableWidgetItem* y2Item = this->item(i, 3);

        //if (!x1Item || !x2Item)
            //continue;
        if (!x1Item || !y1Item || !x2Item || !y2Item)
            continue;

        QVariant x1Var = x1Item->data(Qt::EditRole);
        QVariant y1Var = y1Item->data(Qt::EditRole);
        QVariant x2Var = x2Item->data(Qt::EditRole);
        QVariant y2Var = y2Item->data(Qt::EditRole);

        if (x1Var.isNull() || y1Var.isNull() || x2Var.isNull() || y2Var.isNull())
            continue;

        //QVariant pt1Var = x1Item->data(Qt::UserRole);
        //QVariant pt2Var = x2Item->data(Qt::UserRole);
        //if (pt1Var.isNull() || pt2Var.isNull())
            //continue;

        QPoint pt1(x1Var.toInt(), y1Var.toInt());
        QPoint pt2(x2Var.toInt(), y2Var.toInt());

        PointPair pair(pt1, pt2);
        list.append(pair);
    }
    return list;
}

void PointPairTableWidget::removeSelected()
{
    if (selectionModel()->hasSelection())
    {
        removeRow(selectionModel()->selectedRows()[0].row());
    }
}

void PointPairTableWidget::setLaserPoint(const QPoint& point)
{
    int row = 0;
    if (selectionModel()->hasSelection())
    {
        QModelIndex index = selectionModel()->selectedRows()[0];
        row = index.row();
    }
    else
    {
        if (rowCount() == 0)
        {
            this->insertRow(0);
            row = 0;
        }
        else
        {
            row = rowCount() - 1;
        }
    }

    QTableWidgetItem* itemX = item(row, 0);
    QTableWidgetItem* itemY = item(row, 1);

    if (!itemX)
    {
        itemX = new QTableWidgetItem;
        setItem(row, 0, itemX);
    }
    if (!itemY)
    {
        itemY = new QTableWidgetItem;
        setItem(row, 1, itemY);
    }
    itemX->setData(Qt::EditRole, QString::number(point.x()));
    itemY->setData(Qt::EditRole, QString::number(point.y()));
}

void PointPairTableWidget::setCanvasPoint(const QPoint& point)
{
    int row = 0;
    if (selectionModel()->hasSelection())
    {
        QModelIndex index = selectionModel()->selectedRows()[0];
        row = index.row();
    }
    else
    {
        if (rowCount() == 0)
        {
            this->insertRow(0);
            row = 0;
        }
        else
        {
            row = rowCount() - 1;
        }
    }

    QTableWidgetItem* itemX = item(row, 2);
    QTableWidgetItem* itemY = item(row, 3);

    if (!itemX)
    {
        itemX = new QTableWidgetItem;
        setItem(row, 2, itemX);
    }
    if (!itemY)
    {
        itemY = new QTableWidgetItem;
        setItem(row, 3, itemY);
    }

    qLogD << "canvas point: " << point;
    itemX->setData(Qt::EditRole, QString::number(point.x()));
    itemY->setData(Qt::EditRole, QString::number(point.y()));
}

bool PointPairTableWidget::isEmpty() const
{
    return rowCount() == 0;
}

