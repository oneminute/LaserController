#include "LaserStampBase.h"
#include "LaserStampBasePrivate.h"

#include <QBuffer>
#include <QJsonArray>
#include <QJsonObject>
#include <QPainter>
#include <QtMath>
#include <QTime>

#include "LaserFrame.h"
#include "LaserRing.h"

LaserStampBase::LaserStampBase(LaserStampBasePrivate* ptr, LaserDocument* doc, LaserPrimitiveType type, bool stampIntaglio, QTransform transform,
    int layerIndex, int antiFakeType, int antiFakeLine, bool isAverageDistribute, qreal lineWidth,
    bool surpassOuter, bool surpassInner, bool randomMove)
    :LaserShape(ptr, doc, type, layerIndex, transform)
{
    Q_D(LaserStampBase);
    d->stampIntaglio = stampIntaglio;
    d->antiFakeType = antiFakeType;
    d->antiFakeLine = antiFakeLine;
    d->isAverageDistribute = isAverageDistribute;
    d->antiFakeLineWidth = lineWidth;
    d->surpassOuter = surpassOuter;
    d->surpassInner = surpassInner;
    d->randomMove = randomMove;
    d->m_antiFakeLineSeed = 0;
    d->fingerNoDensityMap = QPixmap();
    d->fingerMapDensity = 0;
}

LaserStampBase::~LaserStampBase()
{
}

void LaserStampBase::setFingerMap(QPixmap map)
{
    Q_D(LaserStampBase);
    d->fingerNoDensityMap = map;
}

QPixmap& LaserStampBase::fingerMap()
{
    Q_D(LaserStampBase);
    return d->fingerNoDensityMap;
}

void LaserStampBase::setFingerMapDensity(qreal density)
{
    Q_D(LaserStampBase);
    d->fingerMapDensity = density;
}

qreal LaserStampBase::fingerMapDensity()
{
    Q_D(LaserStampBase);
    return d->fingerMapDensity;
}

void LaserStampBase::setStampBrush(QPainter* painter, QColor color, QSize size,  QTransform otherTransform, bool isResetColor)
{
    Q_D(LaserStampBase);
    if (d->fingerNoDensityMap.isNull() || d->fingerMapDensity == 0) {
        painter->setBrush(QBrush(color));
    }
    else {
        QPixmap map;
        map = d->fingerNoDensityMap;
        QImage image = map.toImage();
        int tR = qRed(color.rgb());
        int tG = qGreen(color.rgb());
        int tB = qBlue(color.rgb());
        if (isResetColor) {
            for (int row = 0; row < image.width(); row++) {
                for (int col = 0; col < image.height(); col++) {
                    QRgb rgb = image.pixel(row, col);
                    int r = qRed(rgb);
                    int g = qGreen(rgb);
                    int b = qBlue(rgb);
                    int a = qAlpha(rgb);
                    if ((r == 255 && g == 255 && b == 255) || a == 0) {
                        a = 0;
                    }
                    else {
                        a = 255;
                        r = tR;
                        g = tG;
                        b = tB;
                    }
                    image.setPixel(row, col, QColor(r, g, b, a).rgba());
                }
            }
            map = QPixmap::fromImage(image);
        }
        QBrush brush(color, map);
        QTransform t;
        qreal rate = d->fingerMapDensity;
        qreal imageW = image.width();
        qreal imageH = image.height();
        qreal x = d->fingerMapDensity / (imageW / size.width());
        qreal y = d->fingerMapDensity / (imageH / size.height());
        t.scale(x, y);
        
        //t.translate(-d->boundingRect.width() * 0.5, -d->boundingRect.height() * 0.5);
        brush.setTransform(t * otherTransform);
        painter->setBrush(brush);
    }
}

void LaserStampBase::setAntiFakePath(QPainterPath path) {
    Q_D(LaserStampBase);
    d->antiFakePath = path;
    d->path = d->originalPath - d->antiFakePath;
}

bool LaserStampBase::stampIntaglio() {
    Q_D(LaserStampBase);
    return d->stampIntaglio;
}

void LaserStampBase::setStampIntaglio(bool bl) {
    Q_D(LaserStampBase);
    d->stampIntaglio = bl;
}

int LaserStampBase::antiFakeType() {
    Q_D(LaserStampBase);
    return d->antiFakeType;
}

void LaserStampBase::setAntiFakeType(int type) {
    Q_D(LaserStampBase);
    d->antiFakeType = type;
}

int LaserStampBase::antiFakeLine() {
    Q_D(LaserStampBase);
    return d->antiFakeLine;
}

void LaserStampBase::setAntiFakeLine(int count) {
    Q_D(LaserStampBase);
    d->antiFakeLine = count;
}

bool LaserStampBase::isAverageDistribute() {
    Q_D(LaserStampBase);
    return d->isAverageDistribute;
}

void LaserStampBase::setIsAverageDistribute(bool bl) {
    Q_D(LaserStampBase);
    d->isAverageDistribute = bl;
}

qreal LaserStampBase::AntiFakeLineWidth() {
    Q_D(LaserStampBase);
    return d->antiFakeLineWidth;
}

void LaserStampBase::setAntiFakeLineWidth(qreal width) {
    Q_D(LaserStampBase);
    d->antiFakeLineWidth = width;
}

bool LaserStampBase::surpassOuter() {
    Q_D(LaserStampBase);
    return d->surpassOuter;
}

void LaserStampBase::setSurpassOuter(bool bl) {
    Q_D(LaserStampBase);
    d->surpassOuter = bl;
}

bool LaserStampBase::surpassInner() {
    Q_D(LaserStampBase);
    return d->surpassInner;

}

void LaserStampBase::setSurpassInner(bool bl) {
    Q_D(LaserStampBase);
    d->surpassInner = bl;
}

bool LaserStampBase::randomMove() {
    Q_D(LaserStampBase);
    return d->randomMove;
}

void LaserStampBase::setRandomMove(bool bl) {
    Q_D(LaserStampBase);
    d->randomMove = bl;
}

void LaserStampBase::createAntiFakePath(int antiFakeType, int antiFakeLine, bool isAverageDistribute, qreal lineWidth,
    bool surpassOuter, bool surpassInner, bool randomMove)
{
    Q_D(LaserStampBase);
    if (antiFakeLine <= 0 || lineWidth <= 0) {
        setAntiFakePath(QPainterPath());
        return;
    }
    d->antiFakeType = antiFakeType;
    d->antiFakeLine = antiFakeLine;
    d->isAverageDistribute = isAverageDistribute;
    d->antiFakeLineWidth = lineWidth;
    d->surpassOuter = surpassOuter;
    d->surpassInner = surpassInner;
    d->randomMove = randomMove;

    d->antiFakePathData.clear();
    int type = primitiveType();
    if (type == LPT_FRAME || type == LPT_RING) {
        qreal lineWidthRate = 1.5;
        if (d->antiFakeType == 1) {
            lineWidthRate = 3.0;
        }
        
        createAntifakeLineByArc(lineWidthRate);
    }
    else{
        
        createAntifakeLineByBounds();
    }

}

void LaserStampBase::createAntifakeLineByBounds()
{
    Q_D(LaserStampBase);
    d->m_antiFakeLineSeed += 1;

    if (d->m_antiFakeLineSeed >= INT_MAX) {
        d->m_antiFakeLineSeed = 0;
    }
    QRectF bounds = boundingRect();
    int lineType = d->antiFakeType;
    //qreal lineWidth = m_aFWidth->value() * 1000;
    //int lineSize = m_aFLines->value();
    QPainterPath path;
    if (d->antiFakeLineWidth == 0 || d->antiFakeLine == 0) {
        //return QPainterPath();
        setAntiFakePath(path);
        return;
    }

    switch (lineType) {
    case 0: {
        d->antiFakePathData.type = "straight";
        qreal length = (QLineF(bounds.topLeft(), bounds.bottomRight())).length();

        QRectF rect(bounds.topLeft(), QPointF(bounds.left() + length * 1, bounds.top() + d->antiFakeLineWidth));
        QPainterPath lPath;
        lPath.addRect(rect);
        d->antiFakePathData.bounds = rect;

        path = transformAntifakeLineByBounds(lPath, 1.5, 0.2, 0.8);
        break;
    }
    case 1: {
        qreal a = bounds.width() * 0.25;
        d->antiFakePathData.type = "curve";
        d->antiFakePathData.curveAmplitude = a;
        d->antiFakePathData.bounds = bounds;
        d->antiFakePathData.curveBaseLineTL =bounds.topLeft();
        d->antiFakePathData.curveBaseLineTR = bounds.topRight();
        QPainterPath sinPath;
        sinPath = createCurveLine(bounds, a, QLineF(d->antiFakePathData.curveBaseLineTL, d->antiFakePathData.curveBaseLineTR));
        path = transformAntifakeLineByBounds(sinPath, 3, 0.2, 0.8);
        break;
    }
    }
    setAntiFakePath(path);
}

void LaserStampBase::createAntifakeLineByArc(qreal lineWidthRate)
{
    Q_D(LaserStampBase);
    std::uniform_int_distribution<int> u1(0, 100);
    std::default_random_engine e1;
    e1.seed(QTime(0, 0, 0).secsTo(QTime::currentTime()));
    qreal startPercent = u1(e1) / 100.0f;
    int type = primitiveType();
    qreal width;
    if (type == LPT_FRAME) {
        LaserFrame* frame = qgraphicsitem_cast<LaserFrame*>(this);
        width = frame->borderWidth();
        
    }
    else if (type == LPT_RING) {
        LaserRing* ring = qgraphicsitem_cast<LaserRing*>(this);
        
        width = ring->borderWidth();
        
    }
    else {
        setAntiFakePath(QPainterPath());
        return;
    }
    
    QPainterPath basePath;
    QLineF baseLine;
    basePath = createBasePathByArc(width, baseLine);
    QPainterPath path = transformAntifakeLineByArc(basePath, baseLine, lineWidthRate, startPercent);
    setAntiFakePath(path);
}

QPainterPath LaserStampBase::createBasePathByArc(qreal borderWidth, QLineF& baseLine)
{
    Q_D(LaserStampBase);
    QRectF bounds = boundingRect();
    //bool isSurpassOuter = m_aFSurpassOuterCheckbox->isChecked();
    //bool isSurpassInner = m_aFSurpassInnerCheckbox->isChecked();

    qreal height = d->antiFakeLineWidth;
    qreal width, validHeight, validWidth, adpterWidth;
    if (d->surpassOuter && d->surpassInner) {
        width = borderWidth * 4;
        validWidth = borderWidth;
        validHeight = validWidth * 0.25;
        adpterWidth = validWidth;
    }
    else if (d->surpassOuter && !d->surpassInner) {
        width = (borderWidth * 0.9) * 2;
        validWidth = borderWidth * 0.9;
        validHeight = validWidth * 0.25;
        adpterWidth = validWidth;
    }
    else if (!d->surpassOuter && d->surpassInner) {
        width = (borderWidth * 0.9) * 2;
        validWidth = borderWidth * 0.9;
        validHeight = validWidth * 0.25;
        adpterWidth = validWidth;
    }
    else if (!d->surpassOuter && !d->surpassInner) {
        width = borderWidth * 0.8;
        validWidth = borderWidth * 0.8;
        validHeight = validWidth * 0.25;
        adpterWidth = 0;
    }
    QRectF baseRect(bounds.left(), bounds.top(), width, height);
    QLineF l1(baseRect.topLeft(), baseRect.bottomLeft());
    QLineF l2(baseRect.topRight(), baseRect.bottomRight());
    baseLine = QLineF(l1.center(), l2.center());
    QPainterPath path;
    //int lineType = m_aFType->currentIndex();
    switch (d->antiFakeType) {
    case 0: {
        d->antiFakePathData.type = "straight";
        d->antiFakePathData.bounds = baseRect;
        path.addRect(baseRect);
        break;
    }
    case 1: {
        d->antiFakePathData.type = "curve";
        d->antiFakePathData.bounds = baseRect;
        d->antiFakePathData.curveAmplitude = validHeight;
        QPointF tL(l1.center().x() - adpterWidth, l1.center().y());
        QPointF tR(l1.center().x() + validWidth, l1.center().y());
        d->antiFakePathData.curveBaseLineTL = tL;
        d->antiFakePathData.curveBaseLineTR = tR;
        path = createCurveLine(baseRect, validHeight, QLineF(tL, tR));
        break;
    }
    }

    return path;
}

QPainterPath LaserStampBase::createCurveLine(QRectF bounds, qreal a, QLineF line)
{
    Q_D(LaserStampBase);
    QPainterPath path;
    QPainterPath lPath;
    //lPath.moveTo(bounds.topLeft());
    //lPath.lineTo(bounds.topRight());
    lPath.moveTo(line.p1());
    lPath.lineTo(line.p2());
    int size = 50;
    //qreal a = 1000;
    qreal xInterval = bounds.width() / size;
    //qreal width = m_aFWidth->value() * 1000;
    QPointF firstP = lPath.pointAtPercent(0.0);
    QPointF endP = lPath.pointAtPercent(1.0);
    path.moveTo(firstP);
    for (int j = 0; j <= 1; j++) {
        for (int i = 0; i <= size; i++) {
            qreal rate = 1.0f / size * i;
            if (j == 1) {
                rate = 1.0 - 1.0f / size * i;
            }
            QPointF point = lPath.pointAtPercent(rate);
            qreal x = i * xInterval;
            qreal angle = rate * 2 * M_PI;
            qreal y = a * qSin(angle) + d->antiFakeLineWidth * j;
            QPointF resultPoint(point.x(), y + point.y());

            path.lineTo(resultPoint);
        }
        if (j == 0) {
            path.lineTo(QPointF(endP.x(), endP.y() + d->antiFakeLineWidth));
        }
    }
    path.lineTo(firstP);

    return path;
}

QPainterPath LaserStampBase::transformAntifakeLineByBounds(QPainterPath basePath, qreal intervalRate, qreal start, qreal end)
{
    Q_D(LaserStampBase);
    QRectF bounds = boundingRect();

    QPainterPath path, tempL;
    QLineF l1(bounds.topLeft(), bounds.topRight());
    QLineF l2(bounds.bottomLeft(), bounds.bottomRight());
    tempL.moveTo(bounds.topLeft());
    tempL.lineTo(bounds.bottomRight());
    QPainterPath centerLine;
    QPointF cLineA = bounds.center();
    centerLine.moveTo(tempL.pointAtPercent(start));
    centerLine.lineTo(tempL.pointAtPercent(end));

    QPointF c1 = bounds.center();
    QPainterPath topLinePath;

    topLinePath.moveTo(bounds.topLeft());
    topLinePath.lineTo(bounds.topRight());

    //rotate
    std::uniform_int_distribution<int> u1(1, 360);
    std::default_random_engine e1;
    e1.seed(QTime(0, 0, 0).secsTo(QTime::currentTime()) + d->m_antiFakeLineSeed);
    qreal rotateVal = u1(e1);

    QTransform tr;
    tr.rotate(rotateVal);
    basePath = tr.map(basePath);
    topLinePath = tr.map(topLinePath);
    d->antiFakePathData.transformCommonMap.insert("rotate", tr);

    QTransform t;
    QPointF cP = basePath.boundingRect().center();
    t.translate(cLineA.x() - cP.x(), cLineA.y() - cP.y());
    basePath = t.map(basePath);
    topLinePath = t.map(topLinePath);
    QVector2D vec(topLinePath.pointAtPercent(0.0) - topLinePath.pointAtPercent(1.0));
    vec = QVector2D(vec.y(), -vec.x()).normalized();
    d->antiFakePathData.transformCommonMap.insert("translate", t);
    //move
    qreal shorter = bounds.width();
    if (shorter > bounds.height()) {
        shorter = bounds.height();
    }
    int maxLines = centerLine.length() / (d->antiFakeLineWidth * intervalRate);
    int halfLines = maxLines * 0.5;
    if (maxLines < d->antiFakeLine) {
        halfLines = (d->antiFakeLine * 0.5) + 1;
    }
    qreal xL = bounds.width();
    qreal yL = bounds.height();

    if (d->isAverageDistribute) {
        qreal interval = centerLine.length() / (d->antiFakeLine + 1);
        bool isOdd = true;//奇数
        if (d->antiFakeLine % 2 == 0) {
            isOdd = false;
        }
        for (int i = 0; i < d->antiFakeLine; i++) {
            QPainterPath tempPath = basePath;
            qreal val;
            qreal multi = 0;
            if (isOdd) {
                if (i == 0) {
                    path = path.united(tempPath);
                    continue;
                }

                if (i % 2 == 0) {
                    multi = -i / 2;
                }
                else {
                    multi = i / 2 + 1;
                }
            }
            else {
                if (i == 0) {
                    multi = -0.5;
                }
                else if (i == 1) {
                    multi = 0.5;
                }
                else {
                    if (i % 2 == 0) {
                        multi = -(i / 2) - 0.5;
                    }
                    else {
                        multi = (i / 2) + 0.5;
                    }

                }
            }
            val = multi * interval;
            QPointF point(vec.x() * val, vec.y() * val);
            QPointF c2 = tempPath.boundingRect().center();
            QTransform tp;
            tp.translate(point.x() - 0, point.y() - 0);

            tempPath = tp.map(tempPath);
            path = path.united(tempPath);
            QMap< QString, QTransform > transformMap;
            transformMap.insert("translate", tp);
            d->antiFakePathData.transformList.append(transformMap);
        }
    }
    else {
        QList<int> list;
        std::uniform_int_distribution<int> u2(-halfLines, halfLines);
        std::default_random_engine e2;
        e2.seed(QTime(0, 0, 0).secsTo(QTime::currentTime()));
        for (int i = 0; i < d->antiFakeLine; i++) {
            int rand = u2(e2);
            for (int j = 0; j < list.size(); j++) {
                int r = list[j];
                while (r == rand) {
                    rand = u2(e2);
                }
            }
            list.append(rand);
            qreal val = rand * d->antiFakeLineWidth * intervalRate;
            QPainterPath tempPath = basePath;

            QPointF point(vec.x() * val, vec.y() * val);
            QPointF c2 = tempPath.boundingRect().center();
            QTransform tp;
            tp.translate(point.x() - 0, point.y() - 0);
            tempPath = tp.map(tempPath);
            //path.addPath(tempPath);
            path = path.united(tempPath);
            QMap< QString, QTransform > transformMap;
            transformMap.insert("translate", tp);
            d->antiFakePathData.transformList.append(transformMap);
        }
    }
    return path;
}

QPainterPath LaserStampBase::transformAntifakeLineByArc(QPainterPath basePath, QLineF baseLine, qreal lineWidthRate, qreal startPercent)
{
    Q_D(LaserStampBase);
    QRectF outerRect, innerRect;
    int type = primitiveType();
    qreal width, halfWidth;
    QRectF centerRect;
    QPointF center;
    qreal intervalPercent = 1.0 / d->antiFakeLine;

    QPainterPath outerPath, innerPath, centerPath, path;
    if (type == LPT_FRAME) {
        LaserFrame* frame = qgraphicsitem_cast<LaserFrame*>(this);
        innerPath = frame->innerPath();
        innerRect = frame->innerRect();
        width = frame->borderWidth();
        outerPath = frame->outerPath();
        center = innerRect.center();
        halfWidth = width * 0.5;
        centerRect = QRectF(QRectF(QPointF(innerRect.left() - halfWidth, innerRect.top() - halfWidth),
            QPointF(innerRect.right() + halfWidth, innerRect.bottom() + halfWidth)));
        //centerPath.addRect(centerRect);
        centerPath = frame->computeCornerRadius(centerRect.toRect(), frame->cornerRadius(), frame->cornerRadiusType());
    }
    else if (type == LPT_RING) {
        LaserRing* ring = qgraphicsitem_cast<LaserRing*>(this);
        innerPath = ring->innerPath();
        innerRect = ring->innerRect();
        width = ring->borderWidth();
        outerPath = ring->outerPath();
        center = innerRect.center();
        halfWidth = width * 0.5;
        centerRect = QRectF(QRectF(QPointF(innerRect.left() - halfWidth, innerRect.top() - halfWidth),
            QPointF(innerRect.right() + halfWidth, innerRect.bottom() + halfWidth)));
        centerPath.addEllipse(centerRect);
    }
    

    qreal length = innerPath.length();
    int maxLines = qFloor(length / (d->antiFakeLineWidth * lineWidthRate));
    if (maxLines < d->antiFakeLine) {
        maxLines = d->antiFakeLine;
    }

    std::uniform_int_distribution<int> u2(1, maxLines);
    std::default_random_engine e2;
    e2.seed(QTime(0, 0, 0).secsTo(QTime::currentTime()));
    //points percent
    QList<qreal> percentList;
    percentList.append(startPercent);

    if (d->isAverageDistribute) {
        for (int i = 1; i < d->antiFakeLine; i++) {
            qreal val = startPercent + intervalPercent * i;
            if (val > 1) {
                val = val - 1;
            }
            percentList.append(val);
        }
    }
    else {
        for (int i = 1; i < d->antiFakeLine; i++) {
            bool isRepeat = true;
            qreal val;
            while (isRepeat) {
                qreal rVal = u2(e2);
                val = rVal / maxLines;
                for (qreal v : percentList) {
                    if (v == val) {
                        isRepeat = true;
                        break;
                    }
                    else {
                        isRepeat = false;
                    }
                }
            }
            if (!isRepeat) {
                percentList.append(val);
            }
        }
    }
    //QList<QPainterPath> pathList;
    QPainterPath targetPath;
    if (d->surpassOuter && d->surpassInner) {
        targetPath = centerPath;
    }
    else if (d->surpassOuter && !d->surpassInner) {
        targetPath = outerPath;
    }
    else if (!d->surpassOuter && d->surpassInner) {
        targetPath = innerPath;
    }
    else if (!d->surpassOuter && !d->surpassInner) {
        targetPath = centerPath;
    }
    //rotate, translate
    std::uniform_int_distribution<int> u3(0, 360);
    std::default_random_engine e3;
    
    for (int s = 0; s < d->antiFakeLine; s++) {
        QMap<QString, QTransform> transformMap;
        QPainterPath bPath = basePath;
        qreal percent = percentList[s];
        QPointF tPoint = targetPath.pointAtPercent(percent);
        
        //rotate
        if (d->randomMove) {
            e3.seed(QTime(0, 0, 0).secsTo(QTime::currentTime()) + s);
            qreal angle = u3(e3);
            bool bl = true;
            while (bl) {
                if ((angle >= 30 && angle <= 60) || (angle >= 300 && angle <= 330)
                    || (angle >= 120 && angle <= 150) || (angle <= 240 && angle >= 210)) {
                    QTransform rt;
                    rt.rotate(angle);
                    bPath = rt.map(bPath);
                    bl = false;
                    transformMap.insert("rotate", rt);
                }
                else {
                    bl = true;
                    angle = u3(e3);
                }
            }
        }
        else {
            QVector2D arcVect(tPoint - center);
            arcVect = arcVect.normalized();
            QVector2D baseVect(baseLine.p1() - baseLine.p2());
            baseVect = baseVect.normalized();
            qreal radian = qAcos(QVector2D::dotProduct(baseVect, arcVect));
            if (arcVect.y() > 0) {
                radian = -radian;
            }
            QTransform rt;
            rt.rotateRadians(radian);
            bPath = rt.map(bPath);
            transformMap.insert("rotateRadians", rt);
        }
        //translate

        //QPointF tPoint = p->boundingRect().center();
        QPointF diff = tPoint - bPath.boundingRect().center();
        QTransform tt;
        tt.translate(diff.x(), diff.y());
        bPath = tt.map(bPath);
        //path.addPath(bPath);
        path = path.united(bPath);
        transformMap.insert("translate", tt);
        //pathList.append(bPath);
        d->antiFakePathData.transformList.append(transformMap);
    }
    
    return path;
}

void LaserStampBase::stampBaseClone(LaserStampBase* cloneP)
{
    Q_D(LaserStampBase);
    cloneP->setAntiFakePath(d->antiFakePath);
    cloneP->setAntiFakeLine(d->antiFakeLine);
    cloneP->setAntiFakeType(d->antiFakeType);
    cloneP->setAntiFakeLineWidth(d->antiFakeLineWidth);
    cloneP->setIsAverageDistribute(d->isAverageDistribute);
    cloneP->setSurpassInner(d->surpassInner);
    cloneP->setSurpassOuter(d->surpassOuter);
    cloneP->setRandomMove(d->randomMove);
    cloneP->setFingerMap(d->fingerNoDensityMap);
    cloneP->setFingerMapDensity(d->fingerMapDensity);
}

void LaserStampBase::stampBaseToJson(QJsonObject& object)
{
    Q_D(LaserStampBase);
    object.insert("stampIntaglio", d->stampIntaglio);
    object.insert("antiFakeType", d->antiFakeType);
    object.insert("antiFakeLine", d->antiFakeLine);
    object.insert("antiFakeLineWidth", d->antiFakeLineWidth);
    object.insert("isAverageDistribute", d->isAverageDistribute);
    object.insert("surpassInner", d->surpassInner);
    object.insert("surpassOuter", d->surpassOuter);
    object.insert("randomMove", d->randomMove);
    //antifake
    QJsonObject antiFakePathData;
    QJsonArray bounds{
        d->antiFakePathData.bounds.left(), d->antiFakePathData.bounds.top(),
        d->antiFakePathData.bounds.width(), d->antiFakePathData.bounds.height()
    };
    antiFakePathData.insert("bounds", bounds);
    QJsonArray TL{
        d->antiFakePathData.curveBaseLineTL.x(), d->antiFakePathData.curveBaseLineTL.y()
    };
    antiFakePathData.insert("curveBaseLineTL", TL);
    QJsonArray TR{
        d->antiFakePathData.curveBaseLineTR.x(), d->antiFakePathData.curveBaseLineTR.y()
    };
    antiFakePathData.insert("curveBaseLineTR", TR);
    QJsonArray commonArray;
    for (QMap<QString, QTransform>::Iterator i = d->antiFakePathData.transformCommonMap.begin(); i != d->antiFakePathData.transformCommonMap.end(); i++) {
        QTransform t = i.value();
        QJsonArray ta = {
        t.m11(), t.m12(), t.m13(),
        t.m21(), t.m22(), t.m23(),
        t.m31(), t.m32(), t.m33()
        };
        QJsonObject tObject;
        tObject.insert("key", i.key());
        tObject.insert("value", ta);
        commonArray.append(tObject);
    }
    
    QJsonArray tMapArray;
    for (QMap<QString, QTransform> map : d->antiFakePathData.transformList) {
        QJsonArray tArray;
        for (QMap<QString, QTransform>::Iterator i = map.begin(); i != map.end(); i++) {
            QTransform t = i.value();
            QJsonArray ta = {
            t.m11(), t.m12(), t.m13(),
            t.m21(), t.m22(), t.m23(),
            t.m31(), t.m32(), t.m33()
            };
            QJsonObject tObject;
            tObject.insert("key", i.key());
            tObject.insert("value", ta);
            tArray.append(tObject);
        }
        tMapArray.append(tArray);
    }
    antiFakePathData.insert("commonTransforms", commonArray);
    antiFakePathData.insert("transforms", tMapArray);
    antiFakePathData.insert("type", d->antiFakePathData.type);
    antiFakePathData.insert("curveAmplitude", d->antiFakePathData.curveAmplitude);
    object.insert("antiFakePathData", antiFakePathData);
    //fingerprint
    object.insert("fingerMapDensity", d->fingerMapDensity);
    //fingerprintImage
    QByteArray fingerprintImageBits;
    QBuffer fingerprintBuffer(&fingerprintImageBits);
    fingerprintBuffer.open(QIODevice::ReadWrite);
    d->fingerNoDensityMap.save(&fingerprintBuffer, "tiff");
    fingerprintBuffer.close();
    object.insert("fingerNoDensityMap", QLatin1String(fingerprintImageBits.toBase64()));

}