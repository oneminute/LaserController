#include "UndoCommand.h"
#include "scene/LaserPrimitiveGroup.h"
#include "scene/LaserPrimitive.h"
#include "state/StateController.h"
#include "scene/LaserScene.h"
#include "scene/LaserDocument.h"
#include  "util/Utils.h"

SelectionUndoCommand::SelectionUndoCommand(
	LaserViewer * viewer, 
	QMap<QGraphicsItem*, QTransform> undoList,
	QMap<QGraphicsItem*, QTransform> redoList)
{
	m_viewer = viewer;
	m_undoSelectedList = undoList;
	m_redoSelectedList = redoList;
	//m_groupUndoTransform = groupUndoTransform;
	//m_groupRedoTransform = groupRedoTransform;
}

SelectionUndoCommand::~SelectionUndoCommand()
{
	m_viewer = nullptr;
}
void SelectionUndoCommand::undo()
{
	
	handle(m_undoSelectedList);
	/*if (m_reshapeCmd) {
		m_reshapeCmd->undo();
	}
	m_reshapeCmd = nullptr;*/
}

void SelectionUndoCommand::redo()
{
	//qDebug() << m_groupUndoTransform;
	//qDebug() << m_groupRedoTransform;
	if(!m_viewer || !m_viewer->group()|| m_viewer->group()->childItems() == m_redoSelectedList.keys()){
		return;
	}
	//qDebug() << m_viewer->group()->childItems();
	//qDebug() << m_redoSelectedList.keys();
	handle(m_redoSelectedList);
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

void SelectionUndoCommand::sceneTransformToItemTransform(QTransform sceneTransform, QGraphicsItem * item)
{
	item->setTransform(sceneTransform);
	item->setPos(0, 0);
}

void SelectionUndoCommand::handle(QMap<QGraphicsItem*, QTransform> list)
{
	LaserPrimitiveGroup* group = m_viewer->group();
	if (!group) {
		return;
	}
	//m_viewer->clearGroupSelection();
	
	/*for each(LaserPrimitive* primitive in resetList) {
		group->removeFromGroup(primitive);
		primitive->setSelected(false);
	}*/
	m_viewer->clearGroupSelection();
	for (QMap<QGraphicsItem*, QTransform>::Iterator i = list.begin(); i != list.end(); i ++) {
		i.key()->setSelected(true);
		//i.key()->setTransform(i.value());
		sceneTransformToItemTransform(i.value(), i.key());
	}
	m_viewer->onSelectedFillGroup();
	/*for each(LaserPrimitive* primitive in list) {

		primitive->setSelected(true);
		//primitive->setTransform(QTransform());
	}*/
	//sceneTransformToItemTransform(groupTransform, group);
	
	
	/*for each(LaserPrimitive* primitive in list) {

		primitive->setSelected(true);
		//primitive->setTransform(QTransform());
	}*/
	//sceneTransformToItemTransform(groupTransform, group);
	//group->setTransform(groupTransform);
	
	if (!m_viewer->group()->isEmpty() && StateControllerInst.isInState(StateControllerInst.documentIdleState())) {
		emit m_viewer->idleToSelected();
	}
	m_viewer->viewport()->repaint();
}

TranformUndoCommand::TranformUndoCommand(
	LaserViewer* viewer, 
	QMap<QGraphicsItem*, QTransform> undoList, 
	QMap<QGraphicsItem*, QTransform> redoList)
{
	m_viewer = viewer;
	m_undoList = undoList;//sceneTransform
	m_redoList = redoList;
	//m_item = item;
}

TranformUndoCommand::~TranformUndoCommand()
{
}

void TranformUndoCommand::sceneTransformToItemTransform(QTransform sceneTransform, QGraphicsItem* item)
{
	item->setTransform(sceneTransform);
	item->setPos(0, 0);
}

void TranformUndoCommand::undo()
{
	handle(m_undoList);
	/*if (m_item) {
		//m_item->setTransform(m_undoTransform);
		sceneTransformToItemTransform(m_undoTransform, m_item);
	}
	else {
		for each(QGraphicsItem* item in m_viewer->group()->childItems()) {
			item->setTransform(QTransform());
		}
		//m_viewer->group()->setTransform(m_undoTransform);
		sceneTransformToItemTransform(m_undoTransform, m_viewer->group());
	}
	m_viewer->viewport()->repaint();*/
}

void TranformUndoCommand::redo()
{
	handle(m_redoList);
	qDebug() << "TranformUndoCommand::redo()";
	/*if (m_item) {
		if (m_item->sceneTransform() == m_redoTransform) {
			return;
		}
		//m_item->setTransform(m_redoTransform);
		sceneTransformToItemTransform(m_redoTransform, m_item);
	}
	else {
		if (m_viewer->group()->sceneTransform() == m_redoTransform) {
			return;
		}
		for each(QGraphicsItem* item in m_viewer->group()->childItems()) {
			item->setTransform(QTransform());
		}
		//m_viewer->group()->setTransform(m_redoTransform);
		sceneTransformToItemTransform(m_redoTransform, m_viewer->group());
	}
	m_viewer->viewport()->repaint();*/
}

void TranformUndoCommand::handle(QMap<QGraphicsItem*, QTransform> list)
{
	bool isInGroup = true;
	for (QMap<QGraphicsItem*, QTransform>::Iterator i = list.begin(); i != list.end(); i++) {
		if (m_viewer->group()->isAncestorOf(i.key())) {
			if (!isInGroup) {
				qLogW << "in group or not in? confused.";
				return;
			}
			m_viewer->resetGroup();
			sceneTransformToItemTransform(i.value(), i.key());
			m_viewer->onSelectedFillGroup();
		}
		else {
			isInGroup = false;
			sceneTransformToItemTransform(i.value(), i.key());
		}
		
	}
	m_viewer->viewport()->repaint();
}

AddDelUndoCommand::AddDelUndoCommand(LaserScene * scene, QList<QGraphicsItem*> list, bool isDel)
{
	m_scene = scene;
	m_list = list;
	m_isDel = isDel;
	m_viewer = qobject_cast <LaserViewer*>(m_scene->views()[0]);
}

AddDelUndoCommand::~AddDelUndoCommand()
{
}

void AddDelUndoCommand::undo()
{
	if (m_isDel) {
		m_viewer->clearGroupSelection();
		for each(QGraphicsItem* item in m_list) {
			LaserPrimitive* primitive = qgraphicsitem_cast<LaserPrimitive*>(item);
			m_scene->addLaserPrimitive(primitive);
			primitive->setSelected(true);
		}
		
		m_viewer->onSelectedFillGroup();
		//sceneTransformToItemTransform(m_delRedoTransform, m_viewer->group());
		if (StateControllerInst.isInState(StateControllerInst.documentIdleState())) {
			emit m_viewer->idleToSelected();
		}
		
	}
	else {
		m_viewer->clearGroupSelection();
		for each(QGraphicsItem* item in m_list) {
			LaserPrimitive* primitive = qgraphicsitem_cast<LaserPrimitive*>(item);
			sceneTransformToItemTransform(primitive->sceneTransform(), primitive);
			m_scene->removeLaserPrimitive(primitive);
		}
		//恢复之前的group
		for (QMap<QGraphicsItem*, QTransform>::Iterator i = m_selectedBeforeAdd.begin();
			i != m_selectedBeforeAdd.end(); i++) {
			sceneTransformToItemTransform(i.value(), i.key());
			i.key()->setSelected(true);
		}
		m_viewer->onSelectedFillGroup();
		
		if (StateControllerInst.isInState(StateControllerInst.documentSelectedState())) {
			emit m_viewer->selectionToIdle();
		}
		
	}
	m_viewer->viewport()->repaint();
}

void AddDelUndoCommand::redo()
{
	if (m_isDel) {
		m_viewer->clearGroupSelection();
		for each(QGraphicsItem* item in m_list) {
			LaserPrimitive* primitive = qgraphicsitem_cast<LaserPrimitive*>(item);
			sceneTransformToItemTransform(primitive->sceneTransform(), primitive);
			m_scene->removeLaserPrimitive(primitive);
		}
		if (StateControllerInst.isInState(StateControllerInst.documentSelectedState())) {
			emit m_viewer->selectionToIdle();
		}
	}
	else {

		m_selectedBeforeAdd = m_viewer->clearGroupSelection();
		//先添加到scene
		for each(QGraphicsItem* item in m_list) {
			LaserPrimitive* primitive = qgraphicsitem_cast<LaserPrimitive*>(item);
			m_scene->addLaserPrimitive(primitive);
			primitive->setSelected(true);
		}
		m_viewer->onSelectedFillGroup();
		if (StateControllerInst.isInState(StateControllerInst.documentIdleState())) {
			emit m_viewer->idleToSelected();
		}
	}
	m_viewer->viewport()->repaint();
}

void AddDelUndoCommand::sceneTransformToItemTransform(QTransform sceneTransform, QGraphicsItem * item)
{
	if (item == nullptr) {
		return;
	}
	/*QTransform t(
		sceneTransform.m11(), sceneTransform.m12(), sceneTransform.m13(),
		sceneTransform.m21(), sceneTransform.m22(), sceneTransform.m23(),
		0, 0, sceneTransform.m33());*/
	item->setTransform(sceneTransform);
	item->setPos(0, 0);
}

PolygonUndoCommand::PolygonUndoCommand(LaserScene * scene, LaserPrimitive * lastPrimitive, LaserPrimitive * curPrimitive)
{
	m_scene = scene;
	m_lastItem = lastPrimitive;
	m_curItem = curPrimitive;
	m_viewer = qobject_cast <LaserViewer*>( m_scene->views()[0]);
}

PolygonUndoCommand::~PolygonUndoCommand()
{
}

void PolygonUndoCommand::undo()
{
	if (m_curItem) {
		m_viewer->clearGroupSelection();
		sceneTransformToItemTransform(m_curItem->sceneTransform(), m_curItem);
		m_scene->removeLaserPrimitive(m_curItem);
		
	}
	if (m_lastItem) {

		m_scene->addLaserPrimitive(m_lastItem);
		m_lastItem->setSelected(true);
		m_viewer->onSelectedFillGroup();
		/*if (m_curItem == nullptr) {
			m_viewer->onReplaceGroup(m_lastItem);
		}
		else {
			m_curItem->setSelected(m_lastItem);
			m_viewer->onSelectedFillGroup();
		}*/
		
	}
	else {
		if (!m_selectedBeforeAdd.isEmpty()) {
			for (QMap<QGraphicsItem*, QTransform>::Iterator i = m_selectedBeforeAdd.begin();
				i != m_selectedBeforeAdd.end(); i++) {
				sceneTransformToItemTransform(i.value(), i.key());
				i.key()->setSelected(true);
			}
			m_viewer->onSelectedFillGroup();
		}
		
	}
	if (StateControllerInst.isInState(StateControllerInst.documentPrimitivePolygonState())) {
		emit m_viewer->readyPolygon();
	}
	m_viewer->viewport()->repaint();
}

void PolygonUndoCommand::redo()
{

	if (m_lastItem) {
		m_viewer->clearGroupSelection();
		sceneTransformToItemTransform(m_lastItem->sceneTransform(), m_lastItem);
		m_scene->removeLaserPrimitive(m_lastItem);
	}
	else {
		m_selectedBeforeAdd = m_viewer->clearGroupSelection();
	}
	if (m_curItem) {
		m_scene->addLaserPrimitive(m_curItem);
		m_curItem->setSelected(true);
		m_viewer->onSelectedFillGroup();

	}
	m_viewer->viewport()->repaint();
}
void PolygonUndoCommand::sceneTransformToItemTransform(QTransform sceneTransform, QGraphicsItem* item)
{
	item->setTransform(sceneTransform);
	item->setPos(0, 0);
}

MirrorHCommand::MirrorHCommand(LaserViewer * v)
{
	m_viewer = v;
}

MirrorHCommand::~MirrorHCommand()
{
}

void MirrorHCommand::undo()
{
	redo();
}

void MirrorHCommand::redo()
{
	LaserPrimitiveGroup* group = m_viewer->group();
	QTransform t = group->transform();
	QTransform ts = group->sceneTransform();
	QRectF rect = m_viewer->selectedItemsSceneBoundingRect();
	QPointF pos = rect.center();
	QTransform t1 = QTransform(-1, 0, 0, 0, 1, 0, 2 * pos.x(), 0, 1);

	group->setTransform(t * t1);
	m_viewer->viewport()->repaint();
}

MirrorVCommand::MirrorVCommand(LaserViewer * v)
{
	m_viewer = v;
}

MirrorVCommand::~MirrorVCommand()
{
}

void MirrorVCommand::undo()
{
	redo();
}

void MirrorVCommand::redo()
{
	LaserPrimitiveGroup* group = m_viewer->group();
	QTransform t = group->transform();
	QTransform ts = group->sceneTransform();
	QRectF rect = m_viewer->selectedItemsSceneBoundingRect();
	QPointF pos = rect.center();
	QTransform t1 = QTransform(1, 0, 0, 0, -1, 0, 0, 2 * pos.y(), 1);

	group->setTransform(t * t1);
	m_viewer->viewport()->repaint();
}

PasteCommand::PasteCommand(LaserViewer * v, bool isPasteInline, bool isDuplication)
{
	m_viewer = v;
	m_group = m_viewer->group();
	m_scene = m_viewer->scene();
	m_isDuplication = isDuplication;
	m_isPasteInline = isPasteInline;
}

PasteCommand::~PasteCommand()
{
}

void PasteCommand::undo()
{
	
	for each(LaserPrimitive* primitive in m_pasteList) {
		//LaserPrimitive* copyP = new LaserPrimitive(primitive->, primitive->document());
		qDebug() << primitive->sceneTransform();
		utils::sceneTransformToItemTransform(primitive->sceneTransform(), primitive);
		primitive->setSelected(false);
		m_scene->removeLaserPrimitive(primitive);
	}
	m_viewer->clearGroupSelection();
	//恢复之前的group
	for (QMap<QGraphicsItem*, QTransform>::Iterator i = m_pastedBeforeAdd.begin(); i != m_pastedBeforeAdd.end(); i++) {
		
		utils::sceneTransformToItemTransform(i.value(), i.key());
		i.key()->setSelected(true);
	}
	m_viewer->onSelectedFillGroup();
	m_viewer->viewport()->repaint();
}

void PasteCommand::redo()
{
	//第一次进入是创建Item
	if (m_pasteList.isEmpty()) {
		if (!m_isDuplication) {
			for (QMap<LaserPrimitive*, QTransform>::Iterator i = m_viewer->copyedList().begin(); 
				i != m_viewer->copyedList().end(); i++) {
				LaserPrimitive* copyP = i.key()->clone(i.value());
				m_pasteList.append(copyP);
			}
		}
		else {
			for each(QGraphicsItem* item in m_group->childItems()){
				LaserPrimitive* primitive = qgraphicsitem_cast<LaserPrimitive*>(item);
				LaserPrimitive* copyP = primitive->clone(primitive->sceneTransform());
				m_pasteList.append(copyP);
			}
		}
		
		//copy
		if (!m_isDuplication) {
			
			redoImp();
			if (!m_isPasteInline) {
				//设置位置
				QPointF mousePos = m_viewer->mapFromGlobal(QCursor::pos());
				if (mousePos.x() < 0 || mousePos.y() < 0) {
					mousePos = m_viewer->rect().center();
				}
				QPointF initPos = m_viewer->selectedItemsSceneBoundingRect().center();
				QPointF diff = m_viewer->mapToScene(mousePos.toPoint()) - initPos;
				QTransform t = m_group->transform();
				QTransform t1;
				t1.translate(diff.x(), diff.y());
				m_group->setTransform(t * t1);
			}
			
		}
		//duplication
		else {
			redoImp();
		}
		
	}
	else {
		redoImp();
		//copy
		/*if (!m_isDuplication) {
			redoImp();
		}
		//duplication
		else {
			duplicationRedo();
		}*/
	}
	if (!m_viewer->group()->isEmpty() && StateControllerInst.isInState(StateControllerInst.documentIdleState())) {
		emit m_viewer->idleToSelected();
	}
	m_viewer->viewport()->repaint();
}

void PasteCommand::redoImp()
{
	m_pastedBeforeAdd.clear();
	for each(QGraphicsItem* item in m_group->childItems()) {
		m_pastedBeforeAdd.insert(item, item->sceneTransform());
	}
	m_viewer->clearGroupSelection();
	for each(LaserPrimitive* primitive in m_pasteList) {
		//LaserPrimitive* copyP = new LaserPrimitive(primitive->, primitive->document());

		primitive->setSelected(true);
		m_scene->addLaserPrimitive(primitive);
	}
	m_viewer->onSelectedFillGroup();
}

void PasteCommand::duplicationRedo()
{
	
}
