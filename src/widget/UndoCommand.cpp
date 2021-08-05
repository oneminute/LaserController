#include "UndoCommand.h"
#include "scene/LaserPrimitiveGroup.h"
#include "scene/LaserPrimitive.h"
#include "state/StateController.h"

SelectionUndoCommand::SelectionUndoCommand(LaserViewer * viewer, QList<QGraphicsItem*> undoList, QList<QGraphicsItem*> redoList)
{
	m_viewer = viewer;
	m_undoSelectedList = undoList;
	m_redoSelectedList = redoList;
	this->setObsolete(true);
}

SelectionUndoCommand::~SelectionUndoCommand()
{
	m_viewer = nullptr;
}

void SelectionUndoCommand::undo()
{
	handle(m_undoSelectedList);
}

void SelectionUndoCommand::redo()
{
	/*if(m_viewer->group()->childItems() == m_redoSelectedList){
		return;
	}*/
	handle(m_redoSelectedList);
}

void SelectionUndoCommand::handle(QList<QGraphicsItem*> list)
{
	LaserPrimitiveGroup* group = m_viewer->group();
	if (!group) {
		return;
	}
	m_viewer->clearGroupSelection();
	for each(LaserPrimitive* primitive in list) {
		group->removeFromGroup(primitive);
		primitive->setSelected(true);
	}
	m_viewer->onSelectedFillGroup();
	if (!m_viewer->group()->isEmpty() && StateControllerInst.isInState(StateControllerInst.documentIdleState())) {
		emit m_viewer->idleToSelected();
	}
	m_viewer->viewport()->repaint();
}
