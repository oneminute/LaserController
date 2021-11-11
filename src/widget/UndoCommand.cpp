#include "UndoCommand.h"
#include "scene/LaserPrimitiveGroup.h"
#include "scene/LaserPrimitive.h"
#include "state/StateController.h"
#include "scene/LaserScene.h"
#include "scene/LaserDocument.h"
#include  "util/Utils.h"
#include "scene/LaserLayer.h"
#include "LaserApplication.h"
#include "ui/LaserControllerWindow.h"

SelectionUndoCommand::SelectionUndoCommand(
	LaserViewer * viewer, 
	QMap<QGraphicsItem*, QTransform> undoList,
	QMap<QGraphicsItem*, QTransform> redoList)
{
	m_viewer = viewer;
	m_undoSelectedList = undoList;
	m_redoSelectedList = redoList;
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
    //m_viewer->scene()->selectedPrimitives()
    QList<LaserPrimitive*> lPList= m_viewer->scene()->selectedPrimitives();
    QSet<QGraphicsItem*> list;
    QSet<QGraphicsItem*> redoList;
    
    for (QList<LaserPrimitive*>::Iterator i = lPList.end() - 1; i != lPList.begin() - 1; i--) {
        list.insert(*i);
    }

    for (QGraphicsItem* primitive : m_redoSelectedList.keys()) {
        redoList.insert(primitive);
    }

    //qDebug() << list;
    //qDebug() << redoList;
	if(!m_viewer || !m_viewer->group()|| list == redoList){
		return;
	}
	qDebug() << m_viewer->group()->childItems();
	qDebug() << m_redoSelectedList.keys();
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

AddDelUndoCommand::AddDelUndoCommand(LaserScene * scene, QList<QGraphicsItem*> list, bool isDel)
{
	m_scene = scene;
	m_list = list;
	m_isDel = isDel;
	m_viewer = qobject_cast <LaserViewer*>(m_scene->views()[0]);
}

AddDelUndoCommand::AddDelUndoCommand(LaserScene* scene, QList<LaserPrimitive*> list, bool isDel) {
    m_scene = scene;
    m_primitiveList = list;
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
        if (m_primitiveList.isEmpty()) {
            for each(QGraphicsItem* item in m_list) {
                LaserPrimitive* primitive = qgraphicsitem_cast<LaserPrimitive*>(item);
                m_scene->addLaserPrimitive(primitive);
                primitive->setSelected(true);
            }
        }
        else {
            for each(LaserPrimitive* primitive in m_primitiveList) {
                m_scene->addLaserPrimitive(primitive);
                primitive->setSelected(true);
            }
        }

		m_viewer->onSelectedFillGroup();
		if (StateControllerInst.isInState(StateControllerInst.documentIdleState())) {
			emit m_viewer->idleToSelected();
		}
		
	}
	else {
		m_viewer->clearGroupSelection();
        if (m_primitiveList.isEmpty()) {
            for each(QGraphicsItem* item in m_list) {
                LaserPrimitive* primitive = qgraphicsitem_cast<LaserPrimitive*>(item);
                sceneTransformToItemTransform(primitive->sceneTransform(), primitive);
                m_scene->document()->removePrimitive(primitive);
            }
        }
        else {
            for each(LaserPrimitive* primitive in m_primitiveList) {
                sceneTransformToItemTransform(primitive->sceneTransform(), primitive);
                m_scene->document()->removePrimitive(primitive);
            }
        }
		//�ָ�֮ǰ��group
		for (QMap<QGraphicsItem*, QTransform>::Iterator i = m_selectedBeforeAdd.begin();
			i != m_selectedBeforeAdd.end(); i++) {
            LaserPrimitive* p = qgraphicsitem_cast<LaserPrimitive*>(i.key());
			sceneTransformToItemTransform(i.value(), p);
			p->setSelected(true);
            //m_viewer->group()->addToGroup(p);
		}
		m_viewer->onSelectedFillGroup();
		
		if (StateControllerInst.isInState(StateControllerInst.documentSelectedState())) {
			emit m_viewer->selectionToIdle();
		}
		
	}   
	m_viewer->viewport()->repaint();
    emit m_viewer->selectedChange();
}

void AddDelUndoCommand::redo()
{
	if (m_isDel) {
		m_viewer->clearGroupSelection();
        if (m_primitiveList.isEmpty()) {
		    for each(QGraphicsItem* item in m_list) {
			    LaserPrimitive* primitive = qgraphicsitem_cast<LaserPrimitive*>(item);
			    sceneTransformToItemTransform(primitive->sceneTransform(), primitive);
                m_scene->document()->removePrimitive(primitive);
		    }
        }else {
            for each(LaserPrimitive* primitive in m_primitiveList) {
                sceneTransformToItemTransform(primitive->sceneTransform(), primitive);
                m_scene->document()->removePrimitive(primitive);
            }
        }
		if (StateControllerInst.isInState(StateControllerInst.documentSelectedState())) {
			emit m_viewer->selectionToIdle();
		}
	}
	else {

		m_selectedBeforeAdd = m_viewer->clearGroupSelection();
		//����ӵ�scene
        if (m_primitiveList.isEmpty()) {
            for each(QGraphicsItem* item in m_list) {
                LaserPrimitive* primitive = qgraphicsitem_cast<LaserPrimitive*>(item);
                m_scene->addLaserPrimitive(primitive);
                primitive->setSelected(true);
            }
        }
        else {
            for each(LaserPrimitive* primitive in m_primitiveList) {
                m_scene->addLaserPrimitive(primitive);
                primitive->setSelected(true);
            }
        }
		m_viewer->onSelectedFillGroup();
		if (StateControllerInst.isInState(StateControllerInst.documentIdleState())) {
			emit m_viewer->idleToSelected();
		}
	}
    
	m_viewer->viewport()->repaint();
    emit m_viewer->selectedChange();
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
		//m_scene->removeLaserPrimitive(m_curItem);
        m_scene->document()->removePrimitive(m_curItem);
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
		//m_scene->removeLaserPrimitive(m_lastItem);
        m_scene->document()->removePrimitive(m_lastItem);
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
    m_quadTree = v->scene()->quadTreeNode();
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
		//m_scene->removeLaserPrimitive(primitive);
        m_scene->document()->removePrimitive(primitive);
	}
	m_viewer->clearGroupSelection();
	//�ָ�֮ǰ��group
	for (QMap<QGraphicsItem*, QTransform>::Iterator i = m_pastedBeforeAdd.begin(); i != m_pastedBeforeAdd.end(); i++) {
		
		utils::sceneTransformToItemTransform(i.value(), i.key());
		i.key()->setSelected(true);
	}
	m_viewer->onSelectedFillGroup();
	m_viewer->viewport()->repaint();
}

void PasteCommand::redo()
{
	//��һ�ν����Ǵ���Item
    if (m_pasteList.isEmpty()) {
        if (!m_isDuplication) {
            for (QMap<LaserPrimitive*, QTransform>::Iterator i = m_viewer->copyedList().begin();
                i != m_viewer->copyedList().end(); i++) {
                LaserPrimitive* copyP = i.key()->clone(i.value());
                m_pasteList.append(copyP);
            }
        }
        else {
            for each(QGraphicsItem* item in m_group->childItems()) {
                LaserPrimitive* primitive = qgraphicsitem_cast<LaserPrimitive*>(item);
                LaserPrimitive* copyP = primitive->clone(primitive->sceneTransform());
                m_pasteList.append(copyP);
            }
        }
        redoImp(false);
    }
    else {
        redoImp(true);
    }

    
	if (!m_viewer->group()->isEmpty() && StateControllerInst.isInState(StateControllerInst.documentIdleState())) {
		emit m_viewer->idleToSelected();
	}
	m_viewer->viewport()->repaint();
}

void PasteCommand::addImp(bool isAddToTreeNode)
{
	m_pastedBeforeAdd.clear();
	for each(QGraphicsItem* item in m_group->childItems()) {
		m_pastedBeforeAdd.insert(item, item->sceneTransform());
	}
	m_viewer->clearGroupSelection();
	for each(LaserPrimitive* primitive in m_pasteList) {
        if (isAddToTreeNode) {
            m_scene->addLaserPrimitive(primitive);
        }
        else {
            m_scene->addLaserPrimitiveWithoutTreeNode(primitive);
        }
        primitive->setSelected(true);
	}
	m_viewer->onSelectedFillGroup();
}

void PasteCommand::redoImp(bool isRedo)
{
    //copy
    if (!m_isDuplication) {

        if (!m_isPasteInline) {
            addImp();
            QPointF mousePos;
            QPointF initPos = m_viewer->selectedItemsSceneBoundingRect().center();
            if (isRedo) {
                mousePos = m_mouseRedoPos;
            }
            else {
                mousePos = m_viewer->mapFromGlobal(QCursor::pos());
                if (mousePos.x() < 0 || mousePos.y() < 0) {
                    mousePos = m_viewer->rect().center();
                }
                mousePos = m_viewer->mapToScene(mousePos.toPoint());
                m_mouseRedoPos = mousePos;
            }
            QPointF diff = mousePos - initPos;
            QTransform t = m_group->transform();
            QTransform t1;
            t1.translate(diff.x(), diff.y());
            QTransform lastTransform = m_group->sceneTransform();
            m_group->setTransform(t * t1);
            if (!m_viewer->detectBoundsInMaxRegion(m_group->sceneBoundingRect())) {

                m_group->setTransform(lastTransform);
            }
            m_scene->addGroupItemsToTreeNode();
        }
        else {
            addImp(true);
        }

    }
    //duplication
    else {
        addImp(true);
    }
}

/*MirrorACommand::MirrorACommand(LaserViewer * v)
{
    m_viewer = v;
    m_line = qgraphicsitem_cast<LaserLine*>(m_viewer->mirrorLine());
}

MirrorACommand::~MirrorACommand()
{
    m_viewer = nullptr;
    m_line = nullptr;
}

void MirrorACommand::undo()
{
    redo();
}

void MirrorACommand::redo()
{
    LaserPrimitiveGroup* group = m_viewer->group();
    //LaserLine* line = qgraphicsitem_cast<LaserLine*>(m_line);
    QPointF p1 = m_line->line().p1();
    QPointF p2 = m_line->line().p2();
    QVector2D v1(p2 - p1);
    //QVector2D v1(1, 0);
    v1.normalize();
    QVector2D v2(v1.y(), -v1.x());
    QPointF pos = m_line->transform().map(m_line->pos());
    QMatrix mat(v1.x(), v1.y(), v2.x(), v2.y(), p1.x(), p1.y());
    QPointF p = mat.inverted().map(QPointF(1, 0));

    QTransform t(mat.inverted());
    QTransform t1(1, 0, 0, -1, 0, 0);
    QTransform t2 = t.inverted();
    group->setTransform(group->transform() * t * t1 * t2);
    //����ѡ������
    m_viewer->selectedChange();  
    m_viewer->viewport()->repaint();
}*/

LockedCommand::LockedCommand(LaserViewer* v, QCheckBox* locked, Qt::CheckState lastState, QList<LaserPrimitive*> lockedList)
{
    m_viewer = v;
    m_scene = m_viewer->scene();
    m_locked = locked;
    m_lastCheckState = lastState;
    m_curCheckState = m_locked->checkState();
    m_lastLockedList = lockedList;
}

LockedCommand::~LockedCommand()
{
}

void LockedCommand::undo()
{
    LaserPrimitiveGroup* group = m_viewer->group();

    switch (m_lastCheckState)
    {
    case Qt::Checked: {
        for (LaserPrimitive * primitive : m_scene->selectedPrimitives()) {
            primitive->setLocked(true);
            group->removeFromGroup(primitive);

        }
        //m_locked->setCheckState(m_lastCheckState);
        break;
    }
    case Qt::Unchecked: {
        for (LaserPrimitive * primitive : m_scene->selectedPrimitives()) {
            primitive->setLocked(false);
            group->addToGroup(primitive);
        }
        
        break;

    }
    case Qt::PartiallyChecked: {

        for (LaserPrimitive * primitive : m_scene->selectedPrimitives()) {
            bool isLocked = false;
            for (LaserPrimitive* lockedP : m_lastLockedList) {
                if (primitive == lockedP) {
                    primitive->setLocked(true);
                    group->removeFromGroup(primitive);
                    isLocked = true;
                    break;
                }
            }
            if (!isLocked) {
                primitive->setLocked(false);
                group->addToGroup(primitive);
            } 

        }
        //m_locked->setCheckState(Qt::Checked);
        break;
    }
    default:
        break;
    }
    m_locked->setCheckState(m_lastCheckState);
    m_viewer->viewport()->repaint();
    
}

void LockedCommand::redo()
{
    LaserPrimitiveGroup* group = m_viewer->group();

    switch (m_curCheckState)
    {
    case Qt::Checked: {
        for (LaserPrimitive * primitive : m_scene->selectedPrimitives()) {
            primitive->setLocked(true);
            group->removeFromGroup(primitive);

        }
        m_locked->setCheckState(m_curCheckState);
        break;
    }
    case Qt::Unchecked: {
        for (LaserPrimitive * primitive : m_scene->selectedPrimitives()) {
            primitive->setLocked(false);
            group->addToGroup(primitive);
        }
        m_locked->setCheckState(m_curCheckState);
        break;

    }
    case Qt::PartiallyChecked: {
        for (LaserPrimitive * primitive : m_scene->selectedPrimitives()) {
            primitive->setLocked(true);
            group->removeFromGroup(primitive);

        }
        m_locked->setCheckState(Qt::Checked);
        break;
    }
    default:
        break;
    }
    m_viewer->viewport()->repaint();
}

LayerVisibleCommand::LayerVisibleCommand(LaserViewer * v, LaserLayer* layer, bool checked)
{
    m_viewer = v;
    m_group = m_viewer->group();
    m_layer = layer;
    m_checked = checked;
    m_lastIsInGroup = false;
    m_lastSelected = false;
}

void LayerVisibleCommand::undo()
{
    bool undoChecked = !m_checked;
    
    for (LaserPrimitive* primitive : m_layer->primitives()) {
        if (m_lastIsInGroup) {
            m_group->addToGroup(primitive);
        }
        else {
            if (m_group) {
                m_group->removeFromGroup(primitive);
            }
            
        }
        primitive->setSelected(m_lastSelected);
        //primitive->setVisible(undoChecked);
    }
    m_layer->setVisible(undoChecked);
    m_viewer->viewport()->repaint();
}

void LayerVisibleCommand::redo()
{
    for (LaserPrimitive* primitive : m_layer->primitives()) {      
        m_lastSelected = false;
        m_lastIsInGroup = false;
        if (!m_checked) {
            if (m_group) {
                QList<QGraphicsItem*>list = m_group->childItems();
                if (list.contains(primitive)) {
                    m_group->removeFromGroup(primitive);
                    m_lastIsInGroup = true;
                }
                else {
                    m_lastIsInGroup = false;
                }

            }
            if (primitive->isSelected()) {
                primitive->setSelected(false);
                m_lastSelected = true;
            }
            else {
                m_lastSelected = false;
            }
            
        }
        
    }
    m_layer->setVisible(m_checked);
    m_viewer->viewport()->repaint();
}

CornerRadiusCommand::CornerRadiusCommand(LaserViewer * view, LaserRect * rect, LaserDoubleSpinBox* cornerRadius, qreal curVal)
{
    m_view = view;
    m_rect = rect;
    m_cornerRadius = cornerRadius;
    m_window = LaserApplication::mainWindow;
    m_lastRadius = m_window->lastCornerRadiusValue();
    m_curRadius = curVal;
}

CornerRadiusCommand::~CornerRadiusCommand()
{
}

void CornerRadiusCommand::undo()
{
    m_cornerRadius->setValue(m_lastRadius);
    m_rect->setCornerRadius(m_lastRadius);
    m_view->viewport()->repaint();
}

void CornerRadiusCommand::redo()
{
    QRectF bounding = m_rect->boundingRect();
    qreal w = bounding.width();
    qreal h = bounding.height();
    qreal shorter = h;
    if (w < h) {
        shorter = w;
    }
    //qreal val = m_curRadius;
    if (m_curRadius > shorter) {
        m_curRadius = shorter;
        
    }
    m_cornerRadius->setValue(m_curRadius);
    m_rect->setCornerRadius(m_curRadius);
    m_window->setLastCornerRadiusValue(m_curRadius);
    m_view->viewport()->repaint();
}

GroupTransformUndoCommand::GroupTransformUndoCommand(LaserScene * scene, QTransform lastTransform, QTransform curTransform)
    :m_scene(scene), m_lastTransform(lastTransform), m_curTransform(curTransform)
{
    m_viewer = qobject_cast<LaserViewer*>(m_scene->views()[0]);
    m_group = m_viewer->group();
    m_tree = m_scene->quadTreeNode();
    m_lastTransform = lastTransform;
    m_curTransform = curTransform;
    isRedo = false;
}

GroupTransformUndoCommand::~GroupTransformUndoCommand()
{

}

void GroupTransformUndoCommand::undo()
{
    m_group = m_viewer->group();
    m_group->setTransform(m_group->transform()*(m_lastTransform.inverted()*m_curTransform).inverted());
    //updata tree
    m_viewer->updateGroupTreeNode();
    m_viewer->viewport()->repaint();
    isRedo = true;
}

void GroupTransformUndoCommand::redo()
{
    if (!isRedo) {   
        //updata tree
        m_viewer->updateGroupTreeNode();
        return;
    }
    m_group = m_viewer->group();
    m_group->setTransform(m_curTransform);
    //updata tree
    m_viewer->updateGroupTreeNode();
    m_viewer->viewport()->repaint();
}

SingleTransformUndoCommand::SingleTransformUndoCommand(LaserScene * scene, QTransform lastTransform, QTransform curTransform, 
    LaserPrimitive* primitive)
{
    m_scene = scene;
    m_lastTransform = lastTransform;
    m_curTransform = curTransform;
    m_primitive = primitive;
    m_viewer = qobject_cast<LaserViewer*>(m_scene->views()[0]);
    m_tree = m_scene->quadTreeNode();
    isRedo = false;
}

SingleTransformUndoCommand::~SingleTransformUndoCommand()
{
}

void SingleTransformUndoCommand::undo()
{
    isRedo = true;
    //m_primitive->setTransform(m_lastTransform);
    utils::sceneTransformToItemTransform(m_lastTransform, m_primitive);
    m_tree->upDatePrimitive(m_primitive);
    m_viewer->repaint();
}

void SingleTransformUndoCommand::redo()
{
    if (!isRedo) {
        m_tree->upDatePrimitive(m_primitive);
        return;
    }
    utils::sceneTransformToItemTransform(m_curTransform, m_primitive);
    m_tree->upDatePrimitive(m_primitive);
    m_viewer->repaint();
}
