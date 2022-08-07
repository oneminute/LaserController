#pragma once

#include "LaserShape.h"
#include "LaserTextRowPath.h"

class LaserTextPrivate;
class LaserText : public LaserShape
{
	Q_OBJECT
public:
	LaserText(LaserDocument* doc, QPointF startPos, QFont font, qreal spaceY, int alightHType, int alightVType, QTransform transform = QTransform(),
		int layerIndex = 0);
    virtual ~LaserText();

    QRect rect() const;
    QString content() const;
    void setContent(QString c);
    QPainterPath path() const;
    QVector<QLineF> edges();
    void setFont(QFont font);
    QFont font();
    void setAlignH(int a);
    int alignH();
    void setAlignV(int a);
    int alignV();
    QPointF startPos();
    void setSaveTransform(QTransform t);
    QTransform saveTransform();
    //void setAlignType(int type);
    //int alignType();
    void insertContent(QString str, int index);
    //insertIndex,插入到的index
    void addPath(QString content, int insertIndex);
    void delPath(int index);

    qreal spaceY();
    void setSpacceY(qreal space);

    //void modifyLinePathList();
    void modifyPathList();
    QList<LaserTextRowPath> subPathList();

    //virtual QRectF boundingRect() const;
    //virtual QRect sceneBoundingRect() const;
    QRectF originalBoundingRect(qreal extendPixel = 0) const;
	virtual void draw(QPainter* painter);
	virtual LaserPrimitiveType type() { return LPT_TEXT; }
	virtual QString typeName() { return tr("Text"); }
	LaserPrimitive * clone(QTransform t);
    virtual QJsonObject toJson();

    virtual bool isClosed() const;
    virtual QPointF position() const;
    virtual LaserPointListList updateMachiningPoints(ProgressItem* parentProgress);
    virtual LaserLineListList generateFillData(QPointF& lastPoint);

private:
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserText)
	Q_DISABLE_COPY(LaserText)
};

