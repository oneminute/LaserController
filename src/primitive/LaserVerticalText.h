#pragma once

#include "LaserStampText.h"

class LaserVerticalTextPrivate;
class LaserVerticalText : public LaserStampText {
    Q_OBJECT
public:
    LaserVerticalText(LaserDocument* doc, QString content, QSize size,
        QPointF center,bool bold = false, bool italic = false, bool uppercase = false,bool stampIntaglio = false, QString family = "Times New Roman",
        qreal space = 0, QTransform transform = QTransform(), int layerIndex = 0, qreal weight = 0);
    virtual ~LaserVerticalText();
    void computeTextPathProcess();
    void computeTextPath();
    void toCenter();
    virtual void draw(QPainter* painter);
    virtual LaserPrimitiveType type() { return LPT_VERTICALTEXT; }
    virtual QString typeName() { return tr("VerticalText"); }
    virtual QJsonObject toJson();
    virtual QVector<QLine> edges();
    virtual void recompute();
    virtual bool isClosed() const;
    virtual QPointF position() const;
    virtual void setBoundingRectHeight(qreal height);
    virtual void setSpace(qreal space);
    void setTextHeight(qreal height);
    void setTextWidth(qreal width);
private:
    virtual LaserPrimitive * cloneImplement() override;

    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserVerticalText)
    Q_DISABLE_COPY(LaserVerticalText)
};
