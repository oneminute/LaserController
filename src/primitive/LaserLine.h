#pragma once

#include "LaserShape.h"

class LaserLinePrivate;
class LaserLine : public LaserShape
{
    Q_OBJECT
public:
    LaserLine(LaserDocument* doc, QTransform transform = QTransform(),
        int layerIndex = 1);
    LaserLine(const QLine& line, LaserDocument* doc, QTransform transform = QTransform(), int layerIndex = 1);
    virtual ~LaserLine() {}

    QLine line() const;
    void setLine(const QLine& line);
    void setEditingPoint(const QPoint& point);
    QPoint editingPoint() const;

    virtual LaserPointListList updateMachiningPoints(ProgressItem* parentProgress);
    virtual void draw(QPainter* painter);

	//virtual QRect sceneBoundingRect() const;
	//virtual void reShape();
	virtual QJsonObject toJson();
	QVector<QLineF> edges();

    virtual bool isClosed() const;
    virtual QPointF position() const;

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
        bool isPressed) override;
	virtual void sceneKeyPressEvent(
        LaserViewer* viewer,
        QKeyEvent *event);
	virtual void sceneKeyReleaseEvent(
        LaserViewer* viewer,
        QKeyEvent *event);

private:
	virtual LaserPrimitive * cloneImplement() override;

    Q_DISABLE_COPY(LaserLine);
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserLine);

    friend class LaserViewer;
};

