#include "ProgressBarDelegate.h"

#include <QRect>
#include <QtMath>
#include <QProgressBar>

#include "LaserApplication.h"
#include "task/ProgressItem.h"

ProgressBarDelegate::ProgressBarDelegate(QObject* parent)
    : QItemDelegate(parent)
{

}

ProgressBarDelegate::~ProgressBarDelegate()
{
}

void ProgressBarDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (!index.isValid())
        return;

    ProgressItem* item = static_cast<ProgressItem*>(index.internalPointer());
    if (!item)
        return;

    int border = 1;
    QPoint topLeft = option.rect.topLeft() - QPoint(border, border);
    QPoint bottomRight = option.rect.bottomRight() + QPoint(border, border);
    QRect rect(topLeft, bottomRight);

    QStyleOptionProgressBar bar;
    bar.rect = rect;
    bar.state = QStyle::State_Enabled;
    qreal progress = item->progress() * 100;
    bar.progress = qFloor(progress);
    bar.minimum = 0;
    bar.maximum = 100;
    bar.textVisible = true;;
    bar.text = QString("%1%").arg(progress, 5, 'f', 2, QLatin1Char(' '));
    bar.textAlignment = Qt::AlignCenter;

    QProgressBar pbar;
    LaserApplication::style()->drawControl(QStyle::CE_ProgressBar, &bar, painter, &pbar);
}

bool ProgressBarDelegate::editorEvent(QEvent* ev, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (ev->type() == QEvent::MouseButtonDblClick)
        return true;
    return QItemDelegate::editorEvent(ev, model, option, index);
}
