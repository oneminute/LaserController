#include "LaserHeaderView.h"
#include <QPainter>
#include <QLabel>
#include <QApplication>
LaserHeaderView::LaserHeaderView(Qt::Orientation orientation, QWidget *parent)
    :QHeaderView(orientation, parent)
{
}
LaserHeaderView::~LaserHeaderView()
{
}

void LaserHeaderView::paintSection(QPainter * painter, const QRect & rect, int logicalIndex) const
{
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QColor(198, 198, 198));
    painter->drawRect(rect);

    QPixmap map;
    switch (logicalIndex)
    {
    case 0: {
        map = QPixmap(":/ui/icons/images/layerId.png");
        break;
    }
        
    case 1: {
        map = QPixmap(":/ui/icons/images/layer.png");
        break;
    }
        
    case 2: {
        map = QPixmap(":/ui/icons/images/mode.png");
        break;
    }
    case 3: {
        map = QPixmap(":/ui/icons/images/count.png");
        break;
    }
    case 4: {
        map = QPixmap(":/ui/icons/images/speed_power.png");
        break;
    }       
    case 5:
        map = QPixmap(":/ui/icons/images/output.png");
        break;
    case 6:
        map = QPixmap(":/ui/icons/images/visible.png");
        break;
    default:
        break;
    }
    qreal baseSize = 15;
    qreal width = map.width();
    qreal height = map.height();
    qreal scaleW = baseSize / width;
    qreal scaleH = baseSize / height;
    qreal scale = 1;
    if (scaleW > scaleH) {
        scale = scaleW;
    }
    else {
        scale = scaleH;
    }
    width *= scale;
    height *= scale;
    qreal x = rect.x() + (rect.width() - width)*0.5;
    qreal y = rect.y() + (rect.height() - height)*0.5;
    //使用扁平的图片（宽大于高），按照高度为baseSize计算
    map = map.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    painter->drawPixmap(x, y, map);
}
