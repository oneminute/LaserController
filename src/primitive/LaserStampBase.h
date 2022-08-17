#pragma once

#include "LaserShape.h"

class LaserStampBasePrivate;
class LaserStampBase : public LaserShape 
{
    Q_OBJECT
public:
    LaserStampBase(LaserStampBasePrivate* ptr, LaserDocument* doc, LaserPrimitiveType type, bool stampIntaglio, QTransform transform,
        int layerIndex, int antiFakeType = 0, int antiFakeLine = 0, bool isAverageDistribute = false, qreal lineWidth = 0,
        bool surpassOuter = false, bool surpassInner = false, bool randomMove = false);
    ~LaserStampBase();
    void setFingerMap(QPixmap map);
    QPixmap& fingerMap();
    void setFingerMapDensity(qreal density);
    qreal fingerMapDensity();
    void setStampBrush(QPainter* painter, QColor color, QSize size, QTransform otherTransform = QTransform(), bool isResetColor = false);
    virtual void setAntiFakePath(QPainterPath path);
    bool stampIntaglio();
    virtual void setStampIntaglio(bool bl);
    int antiFakeType();
    void setAntiFakeType(int type);
    int antiFakeLine();
    void setAntiFakeLine(int count);
    bool isAverageDistribute();
    void setIsAverageDistribute(bool bl);
    qreal AntiFakeLineWidth();
    void setAntiFakeLineWidth(qreal width);
    bool surpassOuter();
    void setSurpassOuter(bool bl);
    bool surpassInner();
    void setSurpassInner(bool bl);
    bool randomMove();
    void setRandomMove(bool bl);
    void createAntiFakePath(int antiFakeType, int antiFakeLine, bool isAverageDistribute, qreal lineWidth,
    bool surpassOuter = false, bool surpassInner = false, bool randomMove = false);
    void createAntifakeLineByBounds();
    void createAntifakeLineByArc(qreal lineWidthRate);
    QPainterPath createBasePathByArc(qreal borderWidth, QLineF& baseLine);
    QPainterPath createCurveLine(QRectF bounds, qreal a, QLineF line);
    QPainterPath transformAntifakeLineByBounds(QPainterPath basePath, qreal intervalRate, qreal start, qreal end);
    QPainterPath transformAntifakeLineByArc(QPainterPath basePath, QLineF baseLine, qreal lineWidthRate, qreal startPercent);
protected:
    void stampBaseClone(LaserStampBase* sp);
    void stampBaseToJson(QJsonObject& object);

private:
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserStampBase)
        Q_DISABLE_COPY(LaserStampBase)
};

