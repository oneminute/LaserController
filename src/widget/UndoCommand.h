#ifndef UNDOCOMMAND_H
#define UNDOCOMMAND_H
#include <QUndoCommand>
#include <QGraphicsItem>
#include "LaserViewer.h"

class SelectionUndoCommand :public QUndoCommand {
public :
	SelectionUndoCommand(LaserViewer* viewer, 
		QList<QGraphicsItem*> undoList, QTransform groupUndoTransform,
		QList<QGraphicsItem*> redoList, QTransform groupRedoTransform);
	~SelectionUndoCommand();
	virtual void undo() override;
	virtual void redo() override;
	virtual bool mergeWith(const QUndoCommand *command) override;
private:
	LaserViewer* m_viewer;
	QList<QGraphicsItem*> m_undoSelectedList;
	QList<QGraphicsItem*> m_redoSelectedList;
	void handle(QList<QGraphicsItem*> resetList, QList<QGraphicsItem*> list, QTransform groupTransform);
	QTransform m_groupUndoTransform;
	QTransform m_groupRedoTransform;
	//ReshapeUndoCommand* m_reshapeCmd;
};
struct ReshapeUndoPrimitive {
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

class TranformUndoCommand :public QUndoCommand {
public:
	TranformUndoCommand(LaserViewer* viewer, QTransform undoTransform, QTransform redoTransform, LaserPrimitive* item = nullptr);
	~TranformUndoCommand();
	void sceneTransformToItemTransform(QTransform sceneTransform, QGraphicsItem* item);//根据sceneTransfrom转化为Item的transform和position
	virtual void undo() override;
	virtual void redo() override;
private:
	LaserViewer* m_viewer;
	LaserPrimitive* m_item;
	QTransform m_undoTransform;
	QTransform m_redoTransform;
};
#endif // UNDOCOMMAND_H