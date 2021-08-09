#include "UndoCommand.h"
#include "scene/LaserPrimitiveGroup.h"
#include "scene/LaserPrimitive.h"
#include "state/StateController.h"

SelectionUndoCommand::SelectionUndoCommand(
	LaserViewer * viewer, 
	QList<QGraphicsItem*> undoList, QTransform groupUndoTransform,
	QList<QGraphicsItem*> redoList, QTransform groupRedoTransform)
{
	m_viewer = viewer;
	m_undoSelectedList = undoList;
	m_redoSelectedList = redoList;
	m_groupUndoTransform = groupUndoTransform;
	m_groupRedoTransform = groupRedoTransform;
	//m_reshapeCmd = nullptr;
}

SelectionUndoCommand::~SelectionUndoCommand()
{
	m_viewer = nullptr;
}
void SelectionUndoCommand::undo()
{
	
	handle(m_redoSelectedList, m_undoSelectedList, m_groupUndoTransform);
	/*if (m_reshapeCmd) {
		m_reshapeCmd->undo();
	}
	m_reshapeCmd = nullptr;*/
}

void SelectionUndoCommand::redo()
{
	
	if(!m_viewer || !m_viewer->group()|| m_viewer->group()->childItems() == m_redoSelectedList){
		return;
	}
	handle(m_undoSelectedList, m_redoSelectedList, m_groupRedoTransform);
}

bool SelectionUndoCommand::mergeWith(const QUndoCommand * command)
{
	if (!command) {
		return false;
	}
	/*m_reshapeCmd = (ReshapeUndoCommand *)command;
	if (!m_reshapeCmd) {
		return false;
	}*/

	return true;
}

void SelectionUndoCommand::handle(QList<QGraphicsItem*> resetList, QList<QGraphicsItem*> list, QTransform groupTransform)
{
	LaserPrimitiveGroup* group = m_viewer->group();
	if (!group) {
		return;
	}
	//m_viewer->clearGroupSelection();
	group->setTransform(groupTransform);
	for each(LaserPrimitive* primitive in resetList) {
		group->removeFromGroup(primitive);
		primitive->setSelected(false);
	}
	for each(LaserPrimitive* primitive in list) {
		//group->removeFromGroup(primitive);
		primitive->setSelected(true);
	}

	m_viewer->onSelectedFillGroup();
	if (!m_viewer->group()->isEmpty() && StateControllerInst.isInState(StateControllerInst.documentIdleState())) {
		emit m_viewer->idleToSelected();
	}
	m_viewer->viewport()->repaint();
}

ReshapeUndoCommand::ReshapeUndoCommand(LaserViewer * viewer, QMap<LaserPrimitive*, ReshapeUndoPrimitive> undoMap, QMap<LaserPrimitive*, ReshapeUndoPrimitive> redoMap)
{
	m_viewer = viewer;
	m_undoMap = undoMap;
	m_redoMap = redoMap;
}

ReshapeUndoCommand::~ReshapeUndoCommand()
{
}

void ReshapeUndoCommand::undo()
{
	handle(m_undoMap);
}

void ReshapeUndoCommand::redo()
{
	handle(m_redoMap);
}

void ReshapeUndoCommand::handle(QMap<LaserPrimitive*, ReshapeUndoPrimitive>& map)
{
	QList<LaserPrimitive*> list = map.keys();
	if (list.isEmpty()) {
		return;
	}

	for each(LaserPrimitive* item in list) {
		ReshapeUndoPrimitive tp = map[item];
		item->setPos(0, 0);//操作都是在group中做的，所以Iitem的pos应该还是0
		item->setData(tp.path(), tp.allTransform(), tp.transform(), tp.boundingRect());
		//m_viewer->setTransform(QTransform());
	}
	//每一个item都存了group，任取一个即可
	m_viewer->group()->setTransform(map[list[0]].groupTransform());
	m_viewer->viewport()->repaint();
}

bool operator==(const ReshapeUndoPrimitive & t1, const ReshapeUndoPrimitive & t2)
{
	if (t1.path() == t2.path() &&
		t1.allTransform() == t2.allTransform() &&
		t1.transform() == t2.transform() &&
		t1.boundingRect() == t2.boundingRect()&&
		t1.groupTransform() == t2.groupTransform()) {
		return true;
	}
	else {
		return false;
	}
}

TranformUndoCommand::TranformUndoCommand(
	LaserViewer* viewer, QTransform undoTransform, QTransform redoTransform, LaserPrimitive* item)
{
	m_viewer = viewer;
	m_undoTransform = undoTransform;//sceneTransform
	m_redoTransform = redoTransform;
	m_item = item;
}

TranformUndoCommand::~TranformUndoCommand()
{
}

void TranformUndoCommand::sceneTransformToItemTransform(QTransform sceneTransform, QGraphicsItem* item)
{
	QTransform t(
		sceneTransform.m11(), sceneTransform.m12(), sceneTransform.m13(),
		sceneTransform.m21(), sceneTransform.m22(), sceneTransform.m23(),
		0, 0, sceneTransform.m33());
	item->setTransform(t);
	item->setPos(sceneTransform.m31(), sceneTransform.m32());
}

void TranformUndoCommand::undo()
{
	if (m_item) {
		m_item->setTransform(m_undoTransform);
		//sceneTransformToItemTransform(m_undoTransform, m_item);
	}
	else {
		m_viewer->group()->setTransform(m_undoTransform);
		//sceneTransformToItemTransform(m_undoTransform, m_viewer->group());
	}
	m_viewer->viewport()->repaint();
}

void TranformUndoCommand::redo()
{
	if (m_item) {
		if (m_item->transform() == m_redoTransform) {
			return;
		}
		m_item->setTransform(m_redoTransform);
		//sceneTransformToItemTransform(m_undoTransform, m_item);
	}
	else {
		if (m_viewer->group()->transform() == m_redoTransform) {
			return;
		}
		m_viewer->group()->setTransform(m_redoTransform);
		//sceneTransformToItemTransform(m_undoTransform, m_viewer->group());
	}
	m_viewer->viewport()->repaint();
}
