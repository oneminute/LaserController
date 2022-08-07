#include "LaserStampText.h"
#include "LaserStampTextPrivate.h"

#include <QPainter>

#include "scene/LaserDocument.h"
#include "scene/LaserLayer.h"

LaserStampText::LaserStampText(LaserStampTextPrivate* ptr, LaserDocument* doc,LaserPrimitiveType type, QString content, QTransform transform, int layerIndex, 
    QSize size, qreal space, bool bold, bool italic, bool uppercase, bool stampIntaglio, QString family, qreal weight)
    :LaserStampBase(ptr, doc, type, stampIntaglio, transform,layerIndex)
{
    Q_D(LaserStampText);
    d->content = content;
    d->size = size;
    d->space = space;
    d->bold = bold;
    d->italic = italic;
    d->weight = weight;
    d->uppercase = uppercase;
    d->family = family;
    d->fontPiexlSize = size.height();
    //d->stampIntaglio = stampIntaglio;
    setZValue(3);
    
}

LaserStampText::~LaserStampText() {}

void LaserStampText::draw(QPainter* painter)
{
    Q_D(LaserStampText);
    QColor color;
    if (!d->stampIntaglio) {
        //painter->setBrush(QBrush(this->layer()->color()));
        /*if (layer()) {
            color = layer()->color();
            
        }
        else {
            color = Qt::red;
        }*/
        color = d->doc->layers()[d->layerIndex]->color();
    }
    else {
        color = Qt::white;
    }
    
    painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
    QPen pen(color, 1, Qt::SolidLine);
    //pen.setCosmetic(true);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCapStyle(Qt::RoundCap);
    painter->setPen(pen);
    //painter->setBrush(QBrush(color));
    setStampBrush(painter, color, QSize(d->boundingRect.width(), d->boundingRect.height()));
    painter->drawPath(d->path);
}

void LaserStampText::setContent(QString content)
{
    Q_D(LaserStampText);
    d->content = content;
    recompute();
}

QString LaserStampText::getContent()
{
    Q_D(LaserStampText);
    return d->content;
}

void LaserStampText::setBold(bool bold)
{
    Q_D(LaserStampText);
    d->bold = bold;
    recompute();
}

bool LaserStampText::bold()
{
    Q_D(LaserStampText);
    return d->bold;
}

void LaserStampText::setWeight(qreal w)
{
    Q_D(LaserStampText);
    d->weight = w;
    recompute();
}

qreal LaserStampText::weight()
{
    Q_D(LaserStampText);
    return d->weight;
}

void LaserStampText::setItalic(bool italic)
{
    Q_D(LaserStampText);
    d->italic = italic;
    recompute();
}

bool LaserStampText::italic()
{
    Q_D(LaserStampText);
    return d->italic;
}



void LaserStampText::setUppercase(bool uppercase)
{
    Q_D(LaserStampText);
    d->uppercase = uppercase;
    recompute();
}

bool LaserStampText::uppercase()
{
    Q_D(LaserStampText);
    return d->uppercase;
}

void LaserStampText::setFamily(QString family)
{
    Q_D(LaserStampText);
    d->family = family;
    recompute();
}

QString LaserStampText::family()
{
    Q_D(LaserStampText);
    return d->family;
}

qreal LaserStampText::space()
{
    Q_D(LaserStampText);
    return d->space;
}

QSize LaserStampText::textSize()
{
    Q_D(LaserStampText);
    return d->size;
}

