#pragma once

#include "LaserShapePrivate.h"
#include "LaserStampBase.h"

class LaserStampBasePrivate : public LaserShapePrivate
{
    Q_DECLARE_PUBLIC(LaserStampBase)
public:
    LaserStampBasePrivate(LaserStampBase* ptr)
        : LaserShapePrivate(ptr)
    {
    }
    bool stampIntaglio;
    int antiFakeType;
    int antiFakeLine;
    bool isAverageDistribute;
    qreal antiFakeLineWidth;
    bool surpassOuter;
    bool surpassInner;
    bool randomMove;
    int m_antiFakeLineSeed;

    QPainterPath antiFakePath;
    QPainterPath originalPath;
    QPixmap fingerNoDensityMap;
    qreal fingerMapDensity;
    struct AntiFakePathData
    {
        QRectF bounds;
        QMap<QString, QTransform> transformCommonMap;
        QList<QMap<QString, QTransform>> transformList;
        QString type;
        qreal curveAmplitude;
        QPointF curveBaseLineTL;
        QPointF curveBaseLineTR;
        void clear() {
            curveBaseLineTL = QPointF();
            curveBaseLineTR = QPointF();
            bounds = QRectF();
            transformCommonMap.clear();
            transformList.clear();
            type = QString();
            curveAmplitude = 0;
        };
    } antiFakePathData;
};

