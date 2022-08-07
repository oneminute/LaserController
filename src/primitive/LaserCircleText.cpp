#include "LaserCircleText.h"
#include "LaserStampTextPrivate.h"

#include <QJsonArray>
#include <QPainter>
#include <QtMath>

#include "LaserApplication.h"
#include "scene/LaserDocument.h"
#include "scene/LaserLayer.h"
#include "scene/LaserScene.h"
#include "ui/LaserControllerWindow.h"

class LaserCircleTextPrivate : public LaserStampTextPrivate
{
    Q_DECLARE_PUBLIC(LaserCircleText)
public:
    LaserCircleTextPrivate(LaserCircleText* ptr)
        : LaserStampTextPrivate(ptr)
    { 
    }
    //QString content;
    //QSize size;
    qreal angle;
    qreal textEllipse_a;
    qreal textEllipse_b;
    QList<QPainterPath> originalTextPathList;
    QList<QTransform> textTransformList;
    //QPainterPath textPath;
    QPainterPath arcPath;
    qreal minRadian, maxRadian;
    QRectF circleBounds;
    qreal horizontalPathHeight;
    qreal offsetRotateAngle;
    //bool bold;
    //bool italic;
    //bool uppercase;
};

LaserCircleText::LaserCircleText(LaserDocument* doc, QString content, QRectF bounds, qreal angle,
    bool bold, bool italic,bool uppercase, bool stampIntaglio, QString family, qreal sapce,
    bool isInit, qreal maxRadian, qreal minRadian, QSize size, QTransform transform, int layerIndex, qreal weight)
    :LaserStampText(new LaserCircleTextPrivate(this), doc, LPT_CIRCLETEXT, content, transform, layerIndex, size, sapce, bold, italic, uppercase, stampIntaglio, family, weight)
{
    Q_D(LaserCircleText);
    //d->content = content;
    d->offsetRotateAngle = 0;
    //d->stampIntaglio = stampIntaglio;
    d->circleBounds = bounds.toRect();
    //d->boundingRect = bounds.toRect();
    if (!isInit) {
        d->maxRadian = maxRadian;
        d->minRadian = minRadian;
    }
    //d->bold = bold;
    //d->italic = italic;
    //d->uppercase = uppercase;
    setTransform(transform);
    //d->originalBoundingRect = d->boundingRect;    
    d->angle = angle;
    computeTextPath(d->angle, size, isInit);
    //d->path.addRect(d->boundingRect);
    
}

LaserCircleText::~LaserCircleText()
{
}
//needInite 为true，会自动计算textSize， maxRadian，minRadian
void LaserCircleText::computeTextPath(qreal angle, QSize textSize, bool needInit)
{
    Q_D(LaserCircleText);
    if (d->circleBounds.width() <= 0 || d->circleBounds.height() <= 0) {
        return;
    }
    //text height
    setTextSize(textSize, needInit);
    //font
    QFont font;
    font.setWordSpacing(0);
    font.setLetterSpacing(QFont::SpacingType::PercentageSpacing, 0);
    font.setPixelSize(d->size.height());
    font.setBold(d->bold);
    //font.setWeight(81);
    font.setItalic(d->italic);
    font.setFamily(d->family);
    //font.setPointSizeF(d->weight);
    //font.setWeight(d->weight);
    if (d->uppercase) {
        font.setCapitalization(QFont::AllUppercase);
    }
    else {
        font.setCapitalization(QFont::MixedCase);
    }
    d->textTransformList.clear();
    d->originalTextPathList.clear();
    QPainterPath allpath, allOppositePath;
    QList<QPainterPath> originalOppositeTextPathList;
    for (int i = 0; i < d->content.size(); i++) {
        QPainterPath path, oppositePath;
        QChar c = d->content[i];
        path.addText(0, 0, font, c);
        oppositePath.addText(0, 0, font, c);
        QPointF center = oppositePath.boundingRect().center();
        QTransform oppositeT1;
        oppositeT1.scale(-1, 1);
        oppositePath = oppositeT1.map(oppositePath);
        QPointF center1 = oppositePath.boundingRect().center();
        QTransform oppositeT2;
        oppositeT2.translate(center.x() - center1.x(), center.y() - center1.y());
        oppositePath = oppositeT2.map(oppositePath);
        d->originalTextPathList.append(path);
        originalOppositeTextPathList.append(oppositePath);
        allpath.addPath(path);
    }
    d->horizontalPathHeight = allpath.boundingRect().height();
    //d->horizontalPathHeight = d->size.height();
    //angle
    setAngle(angle, needInit);
    //text width
    qreal lengthByPercent = 0;
    QPointF lastP = d->arcPath.pointAtPercent(0);
    for (int i = 0; i < d->content.size(); i++) {
        QPointF p = d->arcPath.pointAtPercent((i+1) * (1.0 / d->content.size()));
        lengthByPercent += QVector2D(p - lastP).length();
        lastP = p;
    }
    //QPointF p = d->arcPath.pointAtPercent(0) - d->arcPath.pointAtPercent(1.0/(d->content.size() - 1));
    qreal distance = lengthByPercent / d->content.size();
    distance = distance - distance * 0.198;
    qreal w = distance - d->space;
    if (w < 1) {
        qreal min = d->minRadian;
        qreal max = d->maxRadian;
        w = 1;
        d->space = distance - w;
    }
    d->size.setWidth(w);
    
    //位移变换
    moveTextToEllipse();
    //变换结束后，加到path中
    int index = 0;
    d->path = QPainterPath();
    for (QPainterPath path : d->originalTextPathList) {
        QTransform t = d->textTransformList[index];
        QPainterPath oppositePath = originalOppositeTextPathList[index];
        d->path.addPath(t.map(path));
        index++;
    }
    d->boundingRect = d->path.boundingRect().toRect();
    d->originalPath = d->path;
}

void LaserCircleText::translateText(QPointF& lastPoint, QPointF& curPoint, qreal interval, qreal index)
{
    Q_D(LaserCircleText);
    //QPointF lastPoint = d->arcPath.pointAtPercent(0.0);
    //qreal interval = 1.0 / (d->content.size() - 1);
    //qreal totalDistance = 0;

    if (index > 0) {
        qreal curInterval = interval * index;
        curPoint = d->arcPath.pointAtPercent(curInterval);

        //cos
        QPointF p = curPoint - d->circleBounds.center();
        QTransform t;
        t.rotate(90);
        p = t.map(p);
        //QVector2D vector = QVector2D().normalized();
        QVector2D verticalVector(p);
        verticalVector = verticalVector.normalized();
        qreal radian = qAcos(QVector2D::dotProduct(verticalVector, QVector2D(1, 0)));
        if (radian > 1) {
            radian = 1;
        }
        else if (radian < -1) {
            radian = -1;
        }
        qreal cosVal = qCos(radian);
        if (curInterval > 0.5) {
            cosVal = -cosVal;

        }
        if (curInterval != 0 && curInterval != 1) {
            if (cosVal > 0) {
                curInterval = curInterval + curInterval * (1-cosVal) * 0.10;
            }
            else {
                curInterval = curInterval + curInterval * (cosVal) * 0.10;
            }
            
        }

        curPoint = d->arcPath.pointAtPercent(curInterval);
        QPointF diffP = curPoint - lastPoint;
        qreal distance = (qSqrt(diffP.x() * diffP.x() + diffP.y() * diffP.y()));
        //lastPoint = curPoint;
        //totalDistance += distance;
    }
}
QTransform LaserCircleText::scaleText(QPainterPath path)
{
    Q_D(LaserCircleText);
    qreal scaleX = d->size.width() / path.boundingRect().width();
    QTransform t1;
    t1.scale(scaleX, 1);
    return t1;
}

QTransform LaserCircleText::rotateText(int i, QPointF textPos)
{
    Q_D(LaserCircleText);
    //Rotate
    qreal rRadian;
    QTransform t2;
    /*qreal intervalRadian = (d->minRadian - d->maxRadian) / (d->content.size() + 1);
    if (d->angle == 360) {
        intervalRadian = (d->minRadian - d->maxRadian) / d->content.size();
    }
    if (d->content.size() == 1) {
        rRadian = 0;
    }
    else {
        if (d->angle != 360) {
            rRadian = d->maxRadian + intervalRadian * (i + 1);
        }
        else {
            rRadian = d->maxRadian + intervalRadian * i;
        }

    }*/
    /*qreal rotateAngle;
    if (d->angle == 360) {
        rotateAngle = ((d->angle / d->content.size()) * i - 180);
    }
    else {
        rotateAngle = -(rRadian / M_PI * 180) + 90;
    }*/
    //根据导数公式求出斜率 -x*b^2 / y*a^2
    qreal a = d->textEllipse_a;
    qreal b = d->textEllipse_b;
    int mode = d->content.size() % 2;
    int c = qFloor(d->content.size() * 0.5);
    QPointF point;
    if (mode == 0) {
        if (i + 1 <= c) {
            point = QPointF(d->originalTextPathList[0].boundingRect().left(), d->originalTextPathList[0].boundingRect().center().y());
        }
        else if (i + 1 > c) {
            point = QPointF(d->originalTextPathList[0].boundingRect().right(), d->originalTextPathList[0].boundingRect().center().y());
        }
    }
    else {
        if (i + 1 <= c) {
            point = QPointF(d->originalTextPathList[0].boundingRect().left(), d->originalTextPathList[0].boundingRect().center().y());
        }
        else if (i + 1 > (c + 1)) {
            point = QPointF(d->originalTextPathList[0].boundingRect().right(), d->originalTextPathList[0].boundingRect().center().y());
        }
        else {
            point = d->originalTextPathList[0].boundingRect().center();
        }
    }
    qreal x = textPos.x() - d->circleBounds.center().x();
    qreal y = -textPos.y() + d->circleBounds.center().y();
    qreal slope = -(b * b * x) / (a * a * y);
    qreal rotateAngle = qAtan(slope) / M_PI * 180;
    if (y < 0) {
        rotateAngle = 180 - rotateAngle;
    }
    else {
        rotateAngle = -rotateAngle;
    }
    t2.rotate(rotateAngle+d->offsetRotateAngle);
    return t2;
}

void LaserCircleText::transformText(QPainterPath path, QPointF textPos, int i)
{
    Q_D(LaserCircleText);
    //将text放置好位置后计算出其center的位置，根据这个位置计算出旋转角度，然后再次计算出最终的矩阵
    QTransform t = scaleText(path) * rotateText(i, textPos);
    QPointF point;
    int mode = d->content.size() % 2;
    int c = qFloor(d->content.size() * 0.5);
    if (mode == 0) {
        if (i + 1 <= c) {
            point = t.map(QPointF(d->originalTextPathList[0].boundingRect().left(), d->originalTextPathList[0].boundingRect().center().y()));
        }
        else if (i + 1 > c) {
            point = t.map(QPointF(d->originalTextPathList[0].boundingRect().right(), d->originalTextPathList[0].boundingRect().center().y()));
        }
    }
    else {
        if (i + 1 <= c) {
            point = t.map(QPointF(d->originalTextPathList[0].boundingRect().left(), d->originalTextPathList[0].boundingRect().center().y()));
        }
        else if (i + 1 > (c + 1)) {
            point = t.map(QPointF(d->originalTextPathList[0].boundingRect().right(), d->originalTextPathList[0].boundingRect().center().y()));
        }
        else {
            point = t.map(d->originalTextPathList[0].boundingRect().center());
        }
    }
    QPointF pos(textPos.x() - point.x(), textPos.y() - point.y());
    QTransform t2 = scaleText(path) * rotateText(i, pos);
    QTransform t1;
    t1.translate(pos.x(), pos.y());
    t = t * t1;
    textPos = t.map(path.boundingRect().center());
    //计算当前位置
    t = scaleText(path) * rotateText(i, textPos);
    QTransform t3;
    point = t.map(d->originalTextPathList[0].boundingRect().center());
    pos = QPointF(textPos.x() - point.x(), textPos.y() - point.y());
    t3.translate(pos.x(), pos.y());
    t = t * t3;
    d->textTransformList.append(t);
}

void LaserCircleText::transformTextByCenter(QPainterPath path, QPointF textPos, int i)
{
    Q_D(LaserCircleText);
    QTransform t = scaleText(path) * rotateText(i, textPos);
    QPointF point = t.map(path.boundingRect().center());
    QPointF pos = QPointF(textPos.x() - point.x(), textPos.y() - point.y());
    QTransform t3;
    t3.translate(pos.x(), pos.y());
    t = t * t3;
    d->textTransformList.append(t);
}

QRectF LaserCircleText::textArcRect()
{
    Q_D(LaserCircleText);
    QRectF rect(d->circleBounds.left() + d->horizontalPathHeight *0.5,
        d->circleBounds.top() + d->horizontalPathHeight *0.5,
        d->circleBounds.width() - d->horizontalPathHeight,
        d->circleBounds.height() - d->horizontalPathHeight);
    /*if (rect.width() > rect.height()) {
        d->textEllipse_a = rect.width() * 0.5;
        d->textEllipse_b = rect.height() * 0.5;
    }
    else {
        d->textEllipse_a = rect.height() * 0.5;
        d->textEllipse_b = rect.width() * 0.5;
    }*/
    d->textEllipse_b = rect.height() * 0.5;
    d->textEllipse_a = rect.width() * 0.5;
    return rect;
}

void LaserCircleText::initAngle()
{
    Q_D(LaserCircleText);
    if (d->angle > 180) {
        qreal diff = (d->angle - 180) * 0.5;
        d->maxRadian = (diff + 180) / 180 * M_PI;
        d->minRadian = -diff / 180 * M_PI;
    }
    else if (d->angle < 180 && d->angle > 0) {
        qreal diff = (180 - d->angle) * 0.5;
        d->maxRadian = (180 - diff) / 180 * M_PI;
        d->minRadian = diff / 180 * M_PI;
    }
    else if (d->angle == 180) {
        d->minRadian = 0;
        d->maxRadian = M_PI;
    }
    else if (d->angle == 0) {
        d->minRadian = M_PI * 0.5;
        d->maxRadian = M_PI * 0.5;
    }
}

void LaserCircleText::setAngle(qreal angle, bool needInit)
{
    Q_D(LaserCircleText);
    if (angle > 360) {
        angle = 360;
    }
    else if (angle < 0) {
        angle = 0;
    }
    qreal diffAngle = angle - d->angle;
    d->angle = angle;
    if (needInit) {
        initAngle();
    }
    else {
        qreal halfDiffRadian = qDegreesToRadians( diffAngle * 0.5);
        d->maxRadian += halfDiffRadian;
        d->minRadian -= halfDiffRadian;
    }
    //需要的圆弧
    QRectF textRect = textArcRect();
    //computeEllipsePoint(d->maxRadian);
    d->arcPath = QPainterPath();
    d->arcPath.arcMoveTo(textArcRect(), qRadiansToDegrees( d->maxRadian) );
    d->arcPath.arcTo(textArcRect(), qRadiansToDegrees(d->maxRadian), -d->angle);
}
void LaserCircleText::setOffsetRotateAngle(qreal offsetAngle)
{
    Q_D(LaserCircleText);
    d->offsetRotateAngle = offsetAngle;
}
void LaserCircleText::setTextSize(QSize size, bool needInit)
{
    Q_D(LaserCircleText);
   
    if (needInit) {
        qreal shorterLine = d->circleBounds.width();
        if (shorterLine > d->circleBounds.height()) {
            shorterLine = d->circleBounds.height();
        }
        d->size.setHeight(shorterLine * 0.22);
    }
    else {
        d->size = size;
    }
}
//radian 的范围在-PI到+PI（-360到360),一次计算中最好只使用一次，不然后加大误差
qreal LaserCircleText::mapToAffineCircleAngle(qreal radian)
{
    Q_D(LaserCircleText);
    qreal oAangle = radian / M_PI * 180.0;
    if (oAangle < 0) {
        oAangle += 360;
    }
    if (oAangle == 0 || oAangle == 90 || oAangle == 180 || oAangle == 270 || oAangle == 360) {
        if (radian < 0) {
            oAangle -= 360;
        }
        return oAangle;
    }
    qreal ratio = d->textEllipse_a / d->textEllipse_b;
    qreal angle = qAtan(qTan(radian) * ratio) / M_PI * 180.0;
    if (angle > 0) {
        if (oAangle > 90) {
            angle += 180;
        }
    }
    else {
        if (oAangle < 270) {
            angle += 180;
        }
        else {
            angle += 360;
        }
        
    }
    if (radian < 0) {
        angle -= 360;
    }
    return angle;
}

void LaserCircleText::moveTextToEllipse()
{
    Q_D(LaserCircleText);
    qreal a = d->textEllipse_a;
    qreal b = d->textEllipse_b;
    qreal h = ((a - b) * (a - b)) / ((a + b) * (a + b));
    qreal P = M_PI * (a + b) *(1 + 3 * h / (10 + qSqrt(4 - 3 * h)));
    bool isCircle = false;
    int textCount = d->content.size();
    qreal averageLength = P / ((360.0 / d->angle) * (d->content.size() - 1));
    qreal shorter = d->circleBounds.width();
    qreal bigger = d->circleBounds.height();
    if (d->circleBounds.width() > d->circleBounds.height()) {
        shorter = d->circleBounds.height();
        bigger = d->circleBounds.width();
    }

    //取最小椭圆直接
    qreal shorterDiameter = a;
    if (shorterDiameter > b) {
        shorterDiameter = b;
    }
    //弧形的起点和终点
    QPointF startPoint = d->arcPath.pointAtPercent(0.0);
    QPointF endPoint = d->arcPath.pointAtPercent(1.0);
    qreal chordLength = QVector2D(startPoint - endPoint).length();
    qreal l = d->size.width() * 1.2;
    qreal ratio = shorter / bigger;
    if (d->angle != 360) {
        computeTextByPercent(d->content.size() - 1);
    }
    else {
        computeTextByPercent(d->content.size());

    }
}

void LaserCircleText::computeTextByPercent(int intervalCount)
{
    Q_D(LaserCircleText);
    d->textTransformList.clear();
    qreal interval = d->arcPath.length() / intervalCount;
    int i = 0;
    for (QPainterPath path : d->originalTextPathList) {
        qreal length = i * interval;
        qreal percent = d->arcPath.percentAtLength(length);
        QPointF textPos = d->arcPath.pointAtPercent(percent);
        if (d->angle == 360) {
            transformTextByCenter(path, textPos, i);
        }
        else {
            //transformText(path, textPos, i);
            transformTextByCenter(path, textPos, i);
        }        
        i++;
    }
}

void LaserCircleText::computeMoveTextPath(qreal diffAngle)
{
    Q_D(LaserCircleText);
    qreal radian = qDegreesToRadians(diffAngle);
    d->maxRadian += radian;
    d->minRadian += radian;
    computeTextPath(d->angle,d->size, false);
}

void LaserCircleText::computeChangeAngle(qreal angle)
{
    Q_D(LaserCircleText);
    computeTextPath(angle, d->size, false);
}

void LaserCircleText::resizeRadian()
{
    Q_D(LaserCircleText);
    qreal range = M_PI * 2;
    if (d->maxRadian > range) {
        d->maxRadian -= range;
        d->minRadian -= range;
    }
    else if (d->minRadian < range || d->maxRadian < 0) {
        d->maxRadian += range;
        d->minRadian += range;
    }
}

QPainterPath * LaserCircleText::textArc()
{
    Q_D(LaserCircleText);
    return &d->arcPath;
}
qreal LaserCircleText::angle()
{
    Q_D(LaserCircleText);
    return d->angle;
}
/*
QPointF LaserCircleText::startPoint()
{
    Q_D(LaserCircleText);
    QPainterPath textEllipse;
    textEllipse.addEllipse(textArcRect());
    QPointF point = textEllipse.pointAtPercent(0.5);
    point = QPointF(point.x() + d->size.height(), point.y());
    return point;
}

QPointF LaserCircleText::endPoint()
{
    Q_D(LaserCircleText);
    QPainterPath textEllipse;
    textEllipse.addEllipse(textArcRect());
    QPointF oP = textEllipse.pointAtPercent(0);
    oP = QPointF(oP.x() - d->size.height(), oP.y());
    return oP;
}

QPointF LaserCircleText::centerPoint()
{
    Q_D(LaserCircleText);
    QPainterPath textEllipse;
    textEllipse.addEllipse(textArcRect());
    QPointF point = textEllipse.pointAtPercent(0.75);
    point = QPointF(point.x(), point.y() + d->size.height());
    return point;
}*/

void LaserCircleText::draw(QPainter * painter)
{
    LaserStampText::draw(painter);
}

LaserPrimitive * LaserCircleText::clone(QTransform t)
{
    Q_D(LaserCircleText);
    LaserCircleText* circleText = new LaserCircleText(d->doc, d->content, d->circleBounds, d->angle, d->bold, d->italic, d->uppercase, d->stampIntaglio, d->family,d->space, false, d->maxRadian, d->minRadian, d->size, transform(), layerIndex());
    stampBaseClone(circleText);
    return circleText;
}

QJsonObject LaserCircleText::toJson()
{
    Q_D(const LaserCircleText);
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
    object.insert("angle", d->angle);
    object.insert("layerIndex", layerIndex());
    object.insert("content", d->content);
    QJsonArray size = {d->size.width(), d->size.height()};
    object.insert("size", size);
    object.insert("maxRadian", d->maxRadian);
    object.insert("minRadian", d->minRadian);
    QJsonArray bounds = { d->circleBounds.x(), d->circleBounds.y(),d->circleBounds.width(), d->circleBounds.height() };
    object.insert("bounds", bounds);
    object.insert("bold", d->bold);
    object.insert("italic", d->italic);
    object.insert("uppercase", d->uppercase);
    object.insert("family", d->family);
    object.insert("stampIntaglio", d->stampIntaglio);
    object.insert("space", d->space);
    stampBaseToJson(object);
    return object;
}

QVector<QLineF> LaserCircleText::edges()
{
    Q_D(LaserCircleText);
    return LaserPrimitive::edges(sceneTransform().map(d->path));
}

bool LaserCircleText::isClosed() const
{
    return false;
}

QPointF LaserCircleText::position() const
{
    return QPointF();
}
//设置space
void LaserCircleText::setBoundingRectWidth(qreal width)
{
    Q_D(LaserCircleText);
    qreal diff = d->circleBounds.width() - width;
    d->circleBounds = QRect(d->circleBounds.x() + diff * 0.5, d->circleBounds.y(),
        width, d->circleBounds.height());
    QPainterPath p;
    p.addRect(d->circleBounds);
    computeTextPath(d->angle, d->size, false);
    LaserApplication::mainWindow->originalBoundsWidth()->setValue(d->circleBounds.width() * 0.001);
}

void LaserCircleText::setBoundingRectHeight(qreal height)
{
    Q_D(LaserCircleText);
    qreal diff = d->circleBounds.height() - height;
    d->circleBounds = QRect(d->circleBounds.x(), d->circleBounds.y() + diff * 0.5,
        d->circleBounds.width(), height);
    QPainterPath p;
    p.addRect(d->circleBounds);
    computeTextPath(d->angle, d->size, false);
    LaserApplication::mainWindow->originalBoundsHeight()->setValue(d->circleBounds.height() * 0.001);
}

void LaserCircleText::recompute()
{
    Q_D(LaserCircleText);
    computeTextPath(d->angle, d->size, false);
}

void LaserCircleText::setSpace(qreal space)
{
    Q_D(LaserCircleText);
    d->space = space;
    computeTextPath(d->angle, d->size, false);
}


QRectF LaserCircleText::circleBounds()
{
    Q_D(LaserCircleText);
    return d->circleBounds;
}

void LaserCircleText::setTextHeight(qreal height)
{
    Q_D(LaserCircleText);
    computeTextPath(d->angle, QSize(d->size.width(), height), false);
}

void LaserCircleText::setTextWidth(qreal width)
{
    //width is auto compute, through the space to change(通过space来修改text的宽度)
    //Q_D(LaserCircleText);
    //computeTextPath(d->angle, QSize(width, d->size.height()), false);
}

