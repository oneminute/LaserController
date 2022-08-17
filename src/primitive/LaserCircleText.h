#pragma once

#include "LaserStampText.h"

class LaserCircleTextPrivate;
class LaserCircleText : public LaserStampText {
    Q_OBJECT
public:
    LaserCircleText(LaserDocument* doc, QTransform transform = QTransform(),
        int layerIndex = 0);
    LaserCircleText(LaserDocument* doc, QString content, QRectF bounds, 
        qreal angle, bool bold = false, bool italic = false, 
        bool uppercase = false, bool stampIntaglio = false, 
        QString family = "Times New Roman",qreal space = 0,
        bool isInit = true, qreal maxRadian = 0, qreal minRadian = 0, 
        QSize size = QSize(), QTransform transform = QTransform(), 
        int layerIndex = 0, qreal weight = 0);
    virtual ~LaserCircleText();
    void computeTextPath(qreal angle, QSize textSize,  bool needInit = true);
    //QPointF computeEllipsePoint(qreal rRadian);
    void translateText(QPointF& lastPoint, QPointF& curPoint, qreal interval, qreal index);
    QTransform scaleText(QPainterPath path);
    QTransform rotateText(int i,QPointF textPos);
    void transformText(QPainterPath path, QPointF textPos, int i);
    void transformTextByCenter(QPainterPath path, QPointF textPos, int i);
    QRectF textArcRect();
    void initAngle();
    void setAngle(qreal angle, bool needInit = true);
    void setOffsetRotateAngle(qreal offsetAngle);
    void setTextSize(QSize size, bool needInit = true);
    qreal mapToAffineCircleAngle(qreal radian);
    void moveTextToEllipse();
    void computeTextByPercent(int intervalCount);
    void computeMoveTextPath(qreal diffAngle);
    void computeChangeAngle(qreal angle);
    void resizeRadian();
    QPainterPath* textArc();
    qreal angle();
    //QPointF startPoint();
    //QPointF endPoint();
    //QPointF centerPoint();
    virtual void draw(QPainter* painter);
    virtual LaserPrimitiveType type() { return LPT_CIRCLETEXT; }
    virtual QString typeName() { return tr("CircleText"); }
    virtual QJsonObject toJson();
    virtual QVector<QLine> edges();

    virtual bool isClosed() const;
    virtual QPointF position() const;
    virtual void setBoundingRectWidth(qreal width);
    virtual void setBoundingRectHeight(qreal height);
    virtual void recompute();
    virtual void setSpace(qreal space);
    QRectF circleBounds();
    void setTextHeight(qreal height);
    void setTextWidth(qreal width);
    
private:
    virtual LaserPrimitive * cloneImplement() override;

    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserCircleText)
    Q_DISABLE_COPY(LaserCircleText)
};

