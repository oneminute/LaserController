#ifndef LASERSCENE_H
#define LASERSCENE_H

#include <QObject>
#include <QGraphicsScene>
#include "scene/LaserBackgroundItem.h"
#include "algorithm/QuadTreeNode.h"

class QPaintEvent;
class QPushButton;

class LaserViewer;
class LaserDocument;
class LaserPrimitive;
class LaserPrimitiveGroup;
class LaserBitmap;

class LaserScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit LaserScene(QObject* parent = nullptr);
    ~LaserScene();

    void setDocument(LaserDocument* doc);
    void clearDocument(bool delDoc = false);
    void addLaserPrimitive(LaserPrimitive* primitive);
	void removeLaserPrimitive(LaserPrimitive* primitive);
	LaserBackgroundItem* backgroundItem() { return m_background; }

    LaserDocument* document() { return m_doc; }

    QList<LaserPrimitive*> selectedPrimitives() const;

	LaserPrimitiveGroup *createItemGroup(const QList<LaserPrimitive*> &items);
	void destroyItemGroup(LaserPrimitiveGroup *group);
	virtual bool eventFilter(QObject *watched, QEvent *event)override;
	bool mousePressDetectBitmap(LaserBitmap* & detected);
	bool mouseMoveBlock();
	virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

	void findSelectedByLine(QRectF rect);
	void findSelectedByBoundingRect(QRectF rect);
    void selectedByBounds(QRectF bounds, QRectF selection, LaserPrimitive* primitive);
    QRectF maxRegion();
    QuadTreeNode* quadTreeNode();
private:
    LaserDocument* m_doc;
	LaserBackgroundItem* m_background;
	bool m_mousePressBlock = false;
	bool m_mouseMoveBlock = false;
	LaserBitmap* m_detectedBitmap = nullptr;
    QuadTreeNode* m_quadTree;
    QRectF m_maxRegion;
};

#endif // LASERSCENE_H