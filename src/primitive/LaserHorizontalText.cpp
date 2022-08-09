#include "LaserHorizontalText.h"
#include "LaserStampTextPrivate.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QPainter>

#include "LaserApplication.h"
#include "common/Config.h"
#include "scene/LaserDocument.h"
#include "scene/LaserLayer.h"
#include "scene/LaserScene.h"
#include "task/ProgressItem.h"
#include "ui/LaserControllerWindow.h"
#include "widget/LaserViewer.h"

class LaserHorizontalTextPrivate : public LaserStampTextPrivate
{
    Q_DECLARE_PUBLIC(LaserHorizontalText)
public:
    LaserHorizontalTextPrivate(LaserHorizontalText* ptr)
        : LaserStampTextPrivate(ptr)
    {
    }
    //QList<QPainterPath> originalTextPathList;
    QPointF center;
};

LaserHorizontalText::LaserHorizontalText(LaserDocument* doc, QString content, QSize size,
    QPointF center, bool bold, bool italic, bool uppercase, bool stampIntaglio, QString family,
    qreal space, QTransform transform, int layerIndex, qreal weight)
    :LaserStampText(new LaserHorizontalTextPrivate(this), doc, LPT_HORIZONTALTEXT,
        content, transform, layerIndex, size, space, bold, italic, uppercase, stampIntaglio, family, weight)
{
    Q_D(LaserHorizontalText);
    //d->stampIntaglio = stampIntaglio;
    setTransform(transform);
    d->center = center;
    computeTextPath();
    d->boundingRect = d->path.boundingRect().toRect();
    //d->originalBoundingRect = d->boundingRect;
    //d->variableBounds = d->boundingRect;
    
}

LaserHorizontalText::~LaserHorizontalText()
{
}

/*void LaserHorizontalText::initTextPath()
{
    Q_D(LaserHorizontalText);
    computeTextPathProcess();
    toBottomLeft();
    d->boundingRect = d->path.boundingRect().toRect();
}*/
void LaserHorizontalText::computeTextPathProcess()
{
    Q_D(LaserHorizontalText);
    QFont font;
    font.setWordSpacing(0);
    font.setLetterSpacing(QFont::SpacingType::PercentageSpacing, 0);
    qreal h = d->size.height();
    font.setPixelSize(d->size.height());
    font.setBold(d->bold);
    font.setItalic(d->italic);
    font.setFamily(d->family);
    //font.setPointSizeF(d->weight);
    //font.setWeight(d->weight);
    QFontMetrics fm(font);
    
    if (d->uppercase) {
        font.setCapitalization(QFont::AllUppercase);
    }
    else {
        font.setCapitalization(QFont::MixedCase);
    }
    d->path = QPainterPath();
    for (int i = 0; i < d->content.size(); i++) {
        QPainterPath path;
        QString c(d->content[i]);
        //scale
        path.addText(0, 0, font, c);
        QTransform t;
        //t.scale(d->size.width() / path.boundingRect().width(), d->size.height() / path.boundingRect().height());
        qreal baseWidth = path.boundingRect().width();
        if (baseWidth < fm.averageCharWidth()) {
            baseWidth = fm.averageCharWidth();
        }
        
        qreal w = d->size.width();
        qreal sX = d->size.width() / baseWidth;
        t.scale(sX, 1);
        path = t.map(path);
        //translate
        QTransform t1;
        //qreal x = i * (d->size.width() + d->space);
        qreal space = d->path.boundingRect().right();
        qreal x = d->path.boundingRect().right() + d->space;
        qreal diffX = x - path.boundingRect().left();
        qreal diffY = 0 - d->size.height();
        t1.translate(diffX, diffY);
        path = t1.map(path);
        d->path.addPath(path);
    }
    
}
//在(0, 0)点调整好间距后(init的时候，单个字的path没有移动位置，最终位置是在all path后移动的)，再移动到位置
void LaserHorizontalText::computeTextPath()
{
    Q_D(LaserHorizontalText);    
    computeTextPathProcess();
    toCenter();
    d->boundingRect = d->path.boundingRect().toRect();  
    //d->originalBoundingRect = d->boundingRect;
    d->originalPath = d->path;
    
}

void LaserHorizontalText::toCenter()
{
    Q_D(LaserHorizontalText);
    QTransform t;
    QPointF diff = d->center - d->path.boundingRect().center();
    t.translate(diff.x(), diff.y());
    d->path = t.map(d->path);
}

void LaserHorizontalText::draw(QPainter * painter)
{
    LaserStampText::draw(painter);
}

LaserPrimitive * LaserHorizontalText::clone()
{
    Q_D(LaserHorizontalText);
    LaserHorizontalText* hText = new LaserHorizontalText(document(), d->content, 
        d->size, d->center, d->bold, d->italic, d->uppercase, d->stampIntaglio,
        d->family, d->space, sceneTransform(), d->layerIndex);
    stampBaseClone(hText);
    return hText;
}

QJsonObject LaserHorizontalText::toJson()
{
    Q_D(const LaserHorizontalText);
    QJsonObject object;
    QTransform pt = QTransform();
    QJsonArray pm = {
        pt.m11(), pt.m12(), pt.m13(),
        pt.m21(), pt.m22(), pt.m23(),
        pt.m31(), pt.m32(), pt.m33()
    };
    object.insert("parentMatrix", pm);
    QTransform t = this->sceneTransform();
    QJsonArray matrix = { t.m11(), t.m12(), t.m13(), t.m21(), t.m22(), t.m23(), t.m31(), t.m32(), t.m33() };
    object.insert("matrix", matrix);
    object.insert("name", name());
    object.insert("className", this->metaObject()->className());
    QJsonArray size = {d->size.width(), d->size.height() };
    object.insert("size", size);
    object.insert("layerIndex", layerIndex());
    object.insert("content", d->content);
    QJsonArray bL = { d->center.x(), d->center.y() };
    object.insert("bottomLeft", bL);
    object.insert("space", d->space);
    object.insert("bold", d->bold);
    object.insert("italic", d->italic);
    object.insert("family", d->family);
    //object.insert("stampIntaglio", d->stampIntaglio);
    stampBaseToJson(object);
    return object;
}

QVector<QLineF> LaserHorizontalText::edges()
{
    Q_D(LaserHorizontalText);
    return LaserPrimitive::edges(sceneTransform().map(d->path));
}

void LaserHorizontalText::recompute()
{
    Q_D(const LaserHorizontalText);
    computeTextPath();
}

bool LaserHorizontalText::isClosed() const
{
    return false;
}

QPointF LaserHorizontalText::position() const
{
    Q_D(const LaserHorizontalText);
    return QPointF();
}
//设置space
void LaserHorizontalText::setBoundingRectWidth(qreal width)
{
    Q_D(LaserHorizontalText);
    
    if (d->content.size() > 1) {
        qreal diff = width - d->boundingRect.width();
        //d->space = (width - d->size.width() * d->content.size()) / (d->content.size() - 1);
        d->space = (width - d->path.boundingRect().width()) / (d->content.size() - 1) + d->space;
        if (d->space < 0) {
            d->space = 0;
            diff = 0;
        }
        LaserApplication::mainWindow->textSpace()->setValue(d->space * 0.001);
        d->center = QPointF(d->center.x(), d->center.y());
        computeTextPath();
    }
    LaserApplication::mainWindow->originalBoundsWidth()->setValue(d->boundingRect.width() * 0.001);
}

void LaserHorizontalText::setSpace(qreal space)
{
    Q_D(LaserHorizontalText);
    d->space = space;
    if (d->space < 0) {
        d->space = 0;
    }
    qreal lastPathWidth = d->path.boundingRect().width();    
    computeTextPath();
    LaserApplication::mainWindow->originalBoundsWidth()->setValue(d->boundingRect.width() * 0.001);
}

void LaserHorizontalText::setTextHeight(qreal height)
{
    Q_D(LaserHorizontalText);
    if (height <= 0) {
        height = 1;
    }
    qreal lastHeight = d->boundingRect.height();
    d->size = QSize(d->size.width(), height);
    computeTextPath();
    //qreal diff = d->boundingRect.height() - lastHeight;
    //d->center = QPointF(d->center.x(), d->center.y() + diff * 0.5);
    //toCenter();
    //d->boundingRect = d->path.boundingRect().toRect();
}

void LaserHorizontalText::setTextWidth(qreal width)
{
    Q_D(LaserHorizontalText);
    if (width <= 0) {
        width = 1;
    }
    //qreal diff = d->content.size() * width + (d->content.size() - 1)*d->space - d->boundingRect.width();
    qreal lasPathWidth = d->path.boundingRect().width();
    d->size = QSize(width, d->size.height());
    computeTextPath();
    LaserApplication::mainWindow->originalBoundsWidth()->setValue(d->boundingRect.width() * 0.001);
}

