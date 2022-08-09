#include "LaserVerticalText.h"
#include "LaserStampTextPrivate.h"

#include <QJsonObject>

#include "LaserApplication.h"
#include "ui/LaserControllerWindow.h"

class LaserVerticalTextPrivate : public LaserStampTextPrivate
{
    Q_DECLARE_PUBLIC(LaserVerticalText)
public:
    LaserVerticalTextPrivate(LaserVerticalText* ptr)
        : LaserStampTextPrivate(ptr)
    {
    }
    //QList<QPainterPath> originalTextPathList;
    QPointF center;
};
LaserVerticalText::LaserVerticalText(LaserDocument* doc, QString content, QSize size, 
    QPointF center, bool bold, bool italic,bool uppercase, bool stampIntaglio, QString family,
    qreal space, QTransform transform, int layerIndex, qreal weight)
    :LaserStampText(new LaserVerticalTextPrivate(this), doc, LPT_VERTICALTEXT,content, transform, layerIndex,
        size,space, bold, italic, uppercase, stampIntaglio, family, weight)
{
    Q_D(LaserVerticalText);
    d->center = center;
    setTransform(transform);
    computeTextPath();
    //d->boundingRect = d->path.boundingRect().toRect();
    //d->originalBoundingRect = d->boundingRect;
}
LaserVerticalText::~LaserVerticalText()
{
}

void LaserVerticalText::computeTextPathProcess()
{
    Q_D(LaserVerticalText);
    QFont font;
    font.setWordSpacing(0);
    font.setLetterSpacing(QFont::SpacingType::PercentageSpacing, 0);
    
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
    //d->originalTextPathList.clear();
    for (int i = 0; i < d->content.size(); i++) {
        QPainterPath path;
        QString c = QString(QChar(d->content[i]));
        qreal baseWidth = fm.width(c);
        if (baseWidth < fm.averageCharWidth()) {
            baseWidth = fm.averageCharWidth();
        }
        //scale
        path.addText(0, 0, font, c);
        QTransform t;
        t.scale(d->size.width() / baseWidth, 1);
        path = t.map(path);
        //d->originalTextPathList.append(path);
        //translate
        QTransform t1;
        //QPointF diff;
        qreal y = d->path.boundingRect().bottom() + d->space;
        //QPointF pos = QPointF(d->originalTextPathList[0].boundingRect().left(), y);
        QPointF pos = QPointF(d->path.boundingRect().center().x(), y);
        QPointF diff = pos - QPointF(path.boundingRect().center().x(), path.boundingRect().top());
        t1.translate(diff.x(), diff.y());
        path = t1.map(path);
        d->path.addPath(path);
    }
    
}

void LaserVerticalText::computeTextPath()
{
    Q_D(LaserVerticalText);
    computeTextPathProcess();
    toCenter();
    d->boundingRect = d->path.boundingRect().toRect();
    d->originalPath = d->path;
}

void LaserVerticalText::toCenter()
{
    Q_D(LaserVerticalText);
    QPointF pos = d->path.boundingRect().center();
    QPointF diff = d->center - pos;
    QTransform t;
    t.translate(diff.x(), diff.y());
    d->path = t.map(d->path);
}

void LaserVerticalText::draw(QPainter * painter)
{
    LaserStampText::draw(painter);
    
}

LaserPrimitive * LaserVerticalText::clone()
{
    Q_D(LaserVerticalText);
    LaserVerticalText* text = new LaserVerticalText(document(), d->content, 
        d->size, d->center,d->bold, d->italic,d->uppercase, d->stampIntaglio, 
        d->family, d->space, sceneTransform(), d->layerIndex);
    stampBaseClone(text);
    return text;
}

QJsonObject LaserVerticalText::toJson()
{
    Q_D(const LaserVerticalText);
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
    QJsonArray size = { d->size.width(), d->size.height() };
    object.insert("size", size);
    object.insert("layerIndex", layerIndex());
    object.insert("content", d->content);
    QJsonArray bL = { d->center.x(), d->center.y() };
    object.insert("topLeft", bL);
    object.insert("space", d->space);
    object.insert("bold", d->bold);
    object.insert("italic", d->italic);
    object.insert("uppercase", d->uppercase);
    object.insert("family", d->family);
    stampBaseToJson(object);
    return object;
}

QVector<QLineF> LaserVerticalText::edges()
{
    Q_D(LaserVerticalText);
    return LaserPrimitive::edges(sceneTransform().map(d->path));
}

void LaserVerticalText::recompute()
{
    Q_D(LaserVerticalText);
    computeTextPath();
}

bool LaserVerticalText::isClosed() const
{
    return false;
}

QPointF LaserVerticalText::position() const
{
    return QPointF();
}
//设置space
void LaserVerticalText::setBoundingRectHeight(qreal height)
{
    Q_D(LaserVerticalText);
    if (d->content.size() > 1) {
        qreal diff = height - d->boundingRect.height();
        d->space = diff / (d->content.size() - 1) + d->space;
        if (d->space < 0) {
            d->space = 0;
            diff = 0;
        }
        LaserApplication::mainWindow->textSpace()->setValue(d->space * 0.001);
        //d->center = QPointF(d->center.x(), d->center.y());
        computeTextPath();
    }
    LaserApplication::mainWindow->originalBoundsHeight()->setValue(d->boundingRect.height() * 0.001);
}

void LaserVerticalText::setSpace(qreal space)
{
    Q_D(LaserVerticalText);
    d->space = space;
    if (d->space < 0) {
        d->space = 0;
    }
    qreal lastPathHeight = d->path.boundingRect().height();
    computeTextPath();
    LaserApplication::mainWindow->originalBoundsHeight()->setValue(d->boundingRect.height() * 0.001);
}

void LaserVerticalText::setTextHeight(qreal height)
{
    Q_D(LaserVerticalText);
    qreal lastPathHeight = d->path.boundingRect().height();
    d->size = QSize(d->size.width(), height);
    computeTextPath();
    LaserApplication::mainWindow->originalBoundsHeight()->setValue(d->boundingRect.height() * 0.001);
}

void LaserVerticalText::setTextWidth(qreal width)
{
    Q_D(LaserVerticalText);
    d->size = QSize(width, d->size.height());
    qreal lastPathWidth = d->path.boundingRect().width();
    //computeTextPathProcess();
    //qreal diff = d->path.boundingRect().width() - lastPathWidth;
    //d->center = QPointF(d->center.x() - diff * 0.5, d->center.y());
    computeTextPath();
}