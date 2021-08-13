#ifndef UNDOCOMMAND_H
#define UNDOCOMMAND_H
#include <QUndoCommand>
#include <QGraphicsItem>
#include "LaserViewer.h"

class SelectionUndoCommand :public QUndoCommand {
public :
	SelectionUndoCommand(LaserViewer * viewer,
		QMap<QGraphicsItem*, QTransform> undoList,
		QMap<QGraphicsItem*, QTransform> redoList);
	~SelectionUndoCommand();
	virtual void undo() override;
	virtual void redo() override;
	virtual bool mergeWith(const QUndoCommand *command) override;
	void sceneTransformToItemTransform(QTransform sceneTransform, QGraphicsItem* item);//根据sceneTransfrom转化为Item的transform和position
private:
	LaserViewer* m_viewer;
	QMap<QGraphicsItem*, QTransform> m_undoSelectedList;
	QMap<QGraphicsItem*, QTransform> m_redoSelectedList;
	void handle(QMap<QGraphicsItem*, QTransform> list);
	QTransform m_groupUndoTransform;
	QTransform m_groupRedoTransform;
	//ReshapeUndoCommand* m_reshapeCmd;
};
class TranformUndoCommand :public QUndoCommand {
public:
	TranformUndoCommand(LaserViewer* viewer,
		QMap<QGraphicsItem*, QTransform> undoList,
		QMap<QGraphicsItem*, QTransform> redoList);
	~TranformUndoCommand();
	void sceneTransformToItemTransform(QTransform sceneTransform, QGraphicsItem* item);//根据sceneTransfrom转化为Item的transform和position
	virtual void undo() override;
	virtual void redo() override;
	void handle(QMap<QGraphicsItem*, QTransform> list);
private:
	LaserViewer * m_viewer;
	//LaserPrimitive* m_item;
	QMap<QGraphicsItem*, QTransform> m_undoList;
	QMap<QGraphicsItem*, QTransform> m_redoList;
	//QTransform m_undoTransform;
	//QTransform m_redoTransform;
};
class AddDelUndoCommand : public QUndoCommand {
public :
	AddDelUndoCommand(LaserScene* scene, QList<QGraphicsItem*> list, bool isDel = false);
	~AddDelUndoCommand();
	virtual void undo() override;
	virtual void redo() override;
	void sceneTransformToItemTransform(QTransform sceneTransform, QGraphicsItem* item);
private:
	LaserScene * m_scene;
	LaserViewer* m_viewer;
	QList<QGraphicsItem*> m_list;
	QMap<QGraphicsItem*, QTransform> m_selectedBeforeAdd;
	//QTransform m_addRedoTransform;
	//QTransform m_delRedoTransform;
	//QMap<QList<QGraphicsItem*>, QTransform> m_selectedBeforeAdd;
	bool m_isDel;
};
class PolygonUndoCommand : public QUndoCommand {
public:
	PolygonUndoCommand(LaserScene* scene, LaserPrimitive* lastPrimitive, LaserPrimitive* curPrimitive);
	~PolygonUndoCommand();
	virtual void undo() override;
	virtual void redo() override;
	void sceneTransformToItemTransform(QTransform sceneTransform, QGraphicsItem* item);
private:
	LaserScene * m_scene;
	LaserPrimitive* m_lastItem;
	LaserPrimitive* m_curItem;
	LaserViewer* m_viewer;
	QMap<QGraphicsItem*, QTransform> m_selectedBeforeAdd;
};
/*struct ReshapeUndoPrimitive {
private:
	QPainterPath m_path;
	QTransform m_allTransform;
	QTransform m_transform;
	QRectF m_boundingRect;
	QTransform m_groupTransform;
public:
	ReshapeUndoPrimitive(
		QPainterPath path,
		QTransform allTransform,
		QTransform transform,
		QRectF boundingRect,
		QTransform groupTransform) {
		m_path = path;
		m_allTransform = allTransform;
		m_transform = transform;
		m_boundingRect = boundingRect;
		m_groupTransform = groupTransform;
	}
	ReshapeUndoPrimitive() {

	}
	~ReshapeUndoPrimitive() {}
	friend bool operator==(const ReshapeUndoPrimitive &t1, const ReshapeUndoPrimitive &t2);
	QPainterPath path() const { return m_path; }
	QTransform allTransform() const { return m_allTransform; }
	QTransform transform() const  { return m_transform; }
	QRectF boundingRect() const { return m_boundingRect; }
	QTransform groupTransform() const { return m_groupTransform; }
};

class ReshapeUndoCommand :public QUndoCommand {
public :
	ReshapeUndoCommand(LaserViewer* viewer, QMap<LaserPrimitive*, ReshapeUndoPrimitive> undoMap, QMap<LaserPrimitive*, ReshapeUndoPrimitive> redoMap);
	~ReshapeUndoCommand();
	virtual void undo() override;
	virtual void redo() override;
private:
	LaserViewer * m_viewer;
	QMap<LaserPrimitive*, ReshapeUndoPrimitive> m_undoMap;
	QMap<LaserPrimitive*, ReshapeUndoPrimitive> m_redoMap;
	void handle(QMap<LaserPrimitive*, ReshapeUndoPrimitive>& map);
};
*/

#endif // UNDOCOMMAND_H