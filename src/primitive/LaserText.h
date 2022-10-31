#pragma once

#include "LaserShape.h"
#include "LaserTextRowPath.h"

class LaserTextPrivate;
class LaserText : public LaserShape
{
	Q_OBJECT
public:
	LaserText(LaserDocument* doc, QTransform transform = QTransform(),
		int layerIndex = 0);
	LaserText(LaserDocument* doc, QPointF startPos, QFont font, qreal spaceY, int alightHType, int alightVType, QTransform transform = QTransform(),
		int layerIndex = 0);
    virtual ~LaserText();

    bool isFirst() const;
    void setIsFirst(bool value);

    QRect rect() const;
    QString content() const;
    void setContent(QString c);
    QPainterPath path() const;
    virtual QVector<QLine> edges();
    void setFont(QFont font);
    QFont font();
    void setAlignH(int a);
    int alignH();
    void setAlignV(int a);
    int alignV();
    QPointF startPos();
    void setStartPos(const QPoint& pos);
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

    int detectInsertIndex(const QPoint& insertPoint);
    void modifyTextCursor(const QPoint& pos);

    QLineF cursorLine() const;
    void setCursorLine(const QLineF& line);

    //virtual QRectF boundingRect() const;
    //virtual QRect sceneBoundingRect() const;
    QRectF originalBoundingRect(qreal extendPixel = 0) const;
	virtual void draw(QPainter* painter);
	virtual LaserPrimitiveType type() { return LPT_TEXT; }
	virtual QString typeName() { return tr("Text"); }
    virtual QJsonObject toJson();

    virtual bool isClosed() const;
    virtual QPointF position() const;
    virtual LaserPointListList updateMachiningPoints(ProgressItem* parentProgress);
    virtual LaserLineListList generateFillData(QPointF& lastPoint);

    void updateText();

protected:
    // the following functions only used in editing mode.
    virtual void sceneMousePressEvent(
        LaserViewer* viewer,
        LaserScene* scene, 
        const QPoint& point,
        QMouseEvent* event) override;
    virtual void sceneMouseMoveEvent(
        LaserViewer* viewer,
        LaserScene* scene,
        const QPoint& point,
        QMouseEvent* event,
        bool isPressed) override;
    virtual void sceneMouseReleaseEvent(
        LaserViewer* viewer,
        LaserScene* scene,
        const QPoint& point,
        QMouseEvent* event,
        bool isPressed,
        QUndoCommand* parentCmd) override;
	virtual void sceneKeyPressEvent(
        LaserViewer* viewer,
        QKeyEvent *event);
	virtual void sceneKeyReleaseEvent(
        LaserViewer* viewer,
        QKeyEvent *event);

    virtual void beginCreatingInternal(QUndoCommand* parentCmd,
        PrimitiveAddingCommand* addingCmd) override;
    virtual void endCreatingInterval(QUndoCommand* parentCmd,
        PrimitiveRemovingCommand* removingCmd) override;

    virtual bool sceneEvent(QEvent* event);

    void inputMethodEvent(QInputMethodEvent * event) override;

    QRectF rectForPosition(int position) const;

public:
    virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;
    Q_INVOKABLE virtual QVariant inputMethodQuery(Qt::InputMethodQuery property, QVariant argument) const;

private:
	virtual LaserPrimitive * cloneImplement() override;

    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserText)
	Q_DISABLE_COPY(LaserText)
};

