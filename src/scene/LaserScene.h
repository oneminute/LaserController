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
    void addLaserPrimitive(LaserPrimitive* primitive, bool ignoreUpdateDocBounding);
    void addLaserPrimitive(LaserPrimitive* primitive, LaserLayer* layer, bool ignoreUpdateDocBounding);
    void addLaserPrimitiveWithoutTreeNode(LaserPrimitive* primitive, bool ignoreUpdateDocBounding);
    void addGroupItemsToTreeNode();
	void removeLaserPrimitive(LaserPrimitive* primitive, bool ignoreUpdateDocBounding);
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

    QSet<LaserPrimitive*> findPrimitivesByRect(const QRectF& rect);
	void findSelectedByLine(QRectF rect);
	void findSelectedByBoundingRect(QRectF rect);
    void findSelectedByRegion(QRectF rect);
    void selectedByBounds(QRectF bounds, QRectF selection, LaserPrimitive* primitive);
    void selectedByLine(QList<QLineF> selectionEdges, QRectF selection, LaserPrimitive* primitive);
    void selectedByRegion(QRectF selection, LaserPrimitive* primitive);
    QRect maxRegion();
    QuadTreeNode* quadTreeNode();
    void updateValidMaxRegionRect();
    void updataValidMaxRegion();
    void updateTree();
    QList<QSet<LaserPrimitive*>*>& joinedGroupList();

private:
    LaserDocument* m_doc;
	LaserBackgroundItem* m_background;
	bool m_mousePressBlock = false;
	bool m_mouseMoveBlock = false;
	LaserBitmap* m_detectedFillSolid = nullptr;
    QuadTreeNode* m_quadTree;
    QRect m_maxRegion;
    QList<QSet<LaserPrimitive*>*> m_joinedGroupList;
};

#endif // LASERSCENE_H