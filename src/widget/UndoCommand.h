#ifndef UNDOCOMMAND_H
#define UNDOCOMMAND_H
#include <QUndoCommand>
#include <QGraphicsItem>
#include "LaserViewer.h"

class SelectionUndoCommand :public QUndoCommand {
public :
	SelectionUndoCommand(LaserViewer* viewer, QList<QGraphicsItem*> undoList, QList<QGraphicsItem*> redoList);
	~SelectionUndoCommand();
	virtual void undo() override;
	virtual void redo() override;
private:
	LaserViewer* m_viewer;
	QList<QGraphicsItem*> m_undoSelectedList;
	QList<QGraphicsItem*> m_redoSelectedList;
	void handle(QList<QGraphicsItem*> list);
};
#endif // UNDOCOMMAND_H