#pragma once

#include "LaserStampText.h"

class LaserHorizontalTextPrivate;
class LaserHorizontalText : public LaserStampText {
    Q_OBJECT
public:
    LaserHorizontalText(LaserDocument* doc, QString content,QSize size,
        QPointF center, bool bold = false, bool italic = false, bool uppercase = false,bool stampIntaglio = false, QString family = "Times New Roman",
        qreal space = 0,  QTransform transform = QTransform(), int layerIndex = 0, qreal weight = 0);
    virtual ~LaserHorizontalText();
    //void initTextPath();
    void computeTextPathProcess();
    void computeTextPath();
    
    void toCenter();
    virtual void draw(QPainter* painter);
    virtual LaserPrimitiveType type() { return LPT_HORIZONTALTEXT; }
    virtual QString typeName() { return tr("HorizontalText"); }
    virtual QJsonObject toJson();
    virtual QVector<QLine> edges();
    virtual void recompute();
    virtual bool isClosed() const;
    virtual QPointF position() const;
    virtual void setBoundingRectWidth(qreal width);
    virtual void setSpace(qreal space);
    void setTextHeight(qreal height);
    void setTextWidth(qreal width);
private:
    virtual LaserPrimitive * cloneImplement() override;

    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserHorizontalText)
    Q_DISABLE_COPY(LaserHorizontalText)
};