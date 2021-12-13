#include "UndoCommand.h"
#include <QAction>
#include <QMessageBox>
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
    
}

void SelectionUndoCommand::redo()
{
    QList<LaserPrimitive*> lPList= m_viewer->scene()->selectedPrimitives();
    QSet<QGraphicsItem*> list;
    QSet<QGraphicsItem*> redoList;
    
    for (QList<LaserPrimitive*>::Iterator i = lPList.end() - 1; i != lPList.begin() - 1; i--) {
        list.insert(*i);
    }

    for (QGraphicsItem* primitive : m_redoSelectedList.keys()) {
        redoList.insert(primitive);
    }
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
    //emit m_viewer->selectedChangedFromMouse();
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
                m_scene->addLaserPrimitive(primitive, true);
                primitive->setSelected(true);
            }
        }
        else {
            for each(LaserPrimitive* primitive in m_primitiveList) {
                m_scene->addLaserPrimitive(primitive, true);
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
                m_scene->removeLaserPrimitive(primitive, true);
            }
        }
        else {
            for each(LaserPrimitive* primitive in m_primitiveList) {
                sceneTransformToItemTransform(primitive->sceneTransform(), primitive);
                m_scene->removeLaserPrimitive(primitive, true);
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
    m_scene->document()->updateDocumentBounding();
	m_viewer->viewport()->repaint();
    emit m_viewer->selectedChangedFromMouse();
}

void AddDelUndoCommand::redo()
{
	if (m_isDel) {
		m_viewer->clearGroupSelection();
        if (m_primitiveList.isEmpty()) {
		    for each(QGraphicsItem* item in m_list) {
			    LaserPrimitive* primitive = qgraphicsitem_cast<LaserPrimitive*>(item);
			    sceneTransformToItemTransform(primitive->sceneTransform(), primitive);
                m_scene->removeLaserPrimitive(primitive, true);
		    }
        }else {
            for each(LaserPrimitive* primitive in m_primitiveList) {
                sceneTransformToItemTransform(primitive->sceneTransform(), primitive);
                m_scene->removeLaserPrimitive(primitive, true);
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
                m_scene->addLaserPrimitive(primitive, true);
                primitive->setSelected(true);
            }
        }
        else {
            for each(LaserPrimitive* primitive in m_primitiveList) {
                m_scene->addLaserPrimitive(primitive, true);
                primitive->setSelected(true);
            }
        }
		m_viewer->onSelectedFillGroup();
		if (StateControllerInst.isInState(StateControllerInst.documentIdleState())) {
			emit m_viewer->idleToSelected();
		}
	}
    m_scene->document()->updateDocumentBounding();
	m_viewer->viewport()->repaint();
    emit m_viewer->selectedChangedFromMouse();
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
        m_scene->removeLaserPrimitive(m_curItem, false);
	}
	if (m_lastItem) {

		m_scene->addLaserPrimitive(m_lastItem, false);
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
        m_scene->removeLaserPrimitive(m_lastItem, false);
	}
	else {
		m_selectedBeforeAdd = m_viewer->clearGroupSelection();
	}
	if (m_curItem) {
		m_scene->addLaserPrimitive(m_curItem, false);
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
        m_scene->removeLaserPrimitive(primitive, true);
	}
	m_viewer->clearGroupSelection();
	//�ָ�֮ǰ��group
	for (QMap<QGraphicsItem*, QTransform>::Iterator i = m_pastedBeforeAdd.begin(); i != m_pastedBeforeAdd.end(); i++) {
		
		utils::sceneTransformToItemTransform(i.value(), i.key());
		i.key()->setSelected(true);
	}
    m_scene->document()->updateDocumentBounding();
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
            m_scene->addLaserPrimitive(primitive, true);
        }
        else {
            m_scene->addLaserPrimitiveWithoutTreeNode(primitive, true);
        }
        primitive->setSelected(true);
	}
    m_scene->document()->updateDocumentBounding();
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
    m_viewer->selectedChangedFromMouse();  
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
        group->setTransform(QTransform());
        for (LaserPrimitive * primitive : m_scene->selectedPrimitives()) {
            primitive->setLocked(false);
            group->addToGroup(primitive);
            
        }        
        break;

    }
    case Qt::PartiallyChecked: {
        for (LaserPrimitive * primitive : m_scene->selectedPrimitives()) {
            //group->setTransform(QTransform());
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
        //group->setTransform(QTransform());
        for (LaserPrimitive * primitive : m_scene->selectedPrimitives()) {
            primitive->setLocked(false);            
        }
        m_viewer->resetGroup();
        m_viewer->onSelectedFillGroup();
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

CornerRadiusCommand::CornerRadiusCommand(LaserViewer * view, QList<LaserPrimitive*>& list, 
    LaserDoubleSpinBox* cornerRadius, qreal curVal, bool _isMulti)
    :m_isMulti(_isMulti), m_view(view), m_rectList(list), 
    m_cornerRadius(cornerRadius), m_curRadius(curVal)
{
    m_window = LaserApplication::mainWindow;
    
    if (m_isMulti) {
        for (LaserPrimitive* primitive : m_rectList) {
            LaserRect* rect = qgraphicsitem_cast<LaserRect*>(primitive);
            m_lastMultiRadiusMap.insert(rect, rect->cornerRadius());
        }
    }
    else {
        LaserRect* rect = qgraphicsitem_cast<LaserRect*>(m_rectList[0]);
        m_lastRadius = rect->cornerRadius();
    }
}

CornerRadiusCommand::~CornerRadiusCommand()
{
}

void CornerRadiusCommand::undo()
{
    if (!m_isMulti) {
        m_cornerRadius->setValue(m_lastRadius);
        m_cornerRadius->setPrefix("");
        for (LaserPrimitive* primitive : m_rectList) {
            LaserRect* rect = qgraphicsitem_cast<LaserRect*>(primitive);
            //prevent the same operation
            if (rect->cornerRadius() == m_lastRadius) {
                return;
            }
            rect->setCornerRadius(m_lastRadius);
        }
    }
    else {
        m_cornerRadius->setValue(0.0);
        m_cornerRadius->setPrefix(QObject::tr("multi"));
        for (QMap<LaserRect*, qreal>::Iterator i = m_lastMultiRadiusMap.begin(); i != m_lastMultiRadiusMap.end(); i++) {
            i.key()->setCornerRadius(i.value());
        }

    }
    
    m_view->viewport()->repaint();
}

void CornerRadiusCommand::redo()
{
    QRect bounding;
    utils::boundingRect(m_rectList, bounding, QRect(), false);
    qreal w = qAbs(bounding.width());
    qreal h = qAbs(bounding.height());
    qreal shorter = h;
    
    if (m_curRadius >= 0) {
        if (w < h) {
            shorter = w;
        }
        if (qAbs(m_curRadius) > shorter) {
            m_curRadius = shorter;

        }
    }
    else {
        if (w < h) {
            shorter = w ;
        }
        if (qAbs(m_curRadius) > shorter) {
            m_curRadius = -shorter * 0.5;

        }
    }
    
    m_cornerRadius->setValue(m_curRadius);
    m_cornerRadius->setPrefix("");
    for (LaserPrimitive* primitive : m_rectList) {
        LaserRect* rect = qgraphicsitem_cast<LaserRect*>(primitive);
        rect->setCornerRadius(m_curRadius);
    }
    m_view->viewport()->repaint();
}

/*RectCommand::RectCommand(LaserViewer * view, QList<LaserPrimitive*>& list, LaserDoubleSpinBox * spinBox, QRectF curRect, bool _isMulti)
    :m_view(view), m_list(list), m_spinBox(spinBox), m_isMulti(_isMulti)
    , m_curRectF(curRect)
{
    LaserPrimitive* firstPrimitive = m_list[0];
    m_type = firstPrimitive->primitiveType();
    if (m_isMulti) {
        for (LaserPrimitive* primitive : m_list) {
            if (m_type == LPT_RECT) {
                LaserRect* rect = qgraphicsitem_cast<LaserRect*>(primitive);
                m_lastMultiMap.insert(rect, rect->rect());
            }
            else if (m_type == LPT_ELLIPSE) {
                LaserEllipse* ellipse = qgraphicsitem_cast<LaserEllipse*>(primitive);
                m_lastMultiMap.insert(ellipse, ellipse->bounds());
            }
            
        }
    }
    else {
        
        if (m_type == LPT_RECT) {
            LaserRect* rect = qgraphicsitem_cast<LaserRect*>(firstPrimitive);
            m_lastRectF = rect->rect();
        }
        else if (m_type == LPT_ELLIPSE) {
            LaserEllipse* ellipse = qgraphicsitem_cast<LaserEllipse*>(firstPrimitive);
            m_lastRectF = ellipse->bounds();
        }
        
    }
}

RectCommand::~RectCommand()
{
}

void RectCommand::undo()
{
}

void RectCommand::redo()
{
    if (m_type == LPT_RECT) {
        for (LaserPrimitive* primitive : m_list) {
            LaserRect* rect = qgraphicsitem_cast<LaserRect*>(primitive);
            rect->setRect(m_curRectF);
        }
    }
    else if (m_type == LPT_ELLIPSE) {
        for (LaserPrimitive* primitive : m_list) {
            LaserEllipse* ellipse = qgraphicsitem_cast<LaserEllipse*>(primitive);
            ellipse->setBounds(m_curRectF);
        }
    }
}*/


GroupTransformUndoCommand::GroupTransformUndoCommand(LaserScene * scene, 
    QTransform lastTransform, QTransform curTransform, bool updataSelectionPanel)
    :m_scene(scene), m_lastTransform(lastTransform), m_curUndoTransform(curTransform)
{
    m_viewer = qobject_cast<LaserViewer*>(m_scene->views()[0]);
    m_group = m_viewer->group();
    m_tree = m_scene->quadTreeNode();
    isRedo = false;
    m_upDataSelectionPanel = updataSelectionPanel;
}

GroupTransformUndoCommand::~GroupTransformUndoCommand()
{

}

void GroupTransformUndoCommand::undo()
{
    m_group = m_viewer->group();
    m_group->setTransform(m_group->transform()*(m_lastTransform.inverted()*m_curUndoTransform).inverted());
    if (m_upDataSelectionPanel) {
        emit m_viewer->selectedChangedFromMouse();
    }
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
    m_group->setTransform(m_group->transform()*(m_curUndoTransform.inverted()*m_lastTransform).inverted());
    if (m_upDataSelectionPanel) {
        emit m_viewer->selectedChangedFromMouse();
    }
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

JoinedGroupCommand::JoinedGroupCommand(LaserViewer * viewer, QAction* _joinedGroupAction, QAction* _joinedUngroupAction, bool _isUngroup)
    :m_viewer(viewer), m_isUngroup(_isUngroup), m_joinedGroupAction(_joinedGroupAction), m_joinedUngroupAction(_joinedUngroupAction)
{
    m_list = m_viewer->group()->childItems();
    int i = m_list.size();
}

JoinedGroupCommand::~JoinedGroupCommand()
{

}

void JoinedGroupCommand::undo()
{
    if (!m_isUngroup) {
        handleUnGroup();
        
    }
    else {
        handleGroup();
    }
    emit LaserApplication::mainWindow->joinedGroupChanged();
}

void JoinedGroupCommand::redo()
{
    if (!m_isUngroup) {
        
        handleGroup();
    }
    else {
        handleUnGroup();
    }
    emit LaserApplication::mainWindow->joinedGroupChanged();
}

void JoinedGroupCommand::handleGroup()
{
    //QRect sceneBoundingRect;
    //utils::boundingRect(m_list, sceneBoundingRect, QRect(), false);
    for (QGraphicsItem* item : m_list) {
        LaserPrimitive* primitive = qgraphicsitem_cast<LaserPrimitive*>(item);
        primitive->setJoinedGroup(true);
        //primitive->setJoinedGroupSceneBoundingRect(sceneBoundingRect);
    }
    m_viewer->viewport()->repaint();
    m_joinedGroupAction->setEnabled(false);
    m_joinedUngroupAction->setEnabled(true);
}

void JoinedGroupCommand::handleUnGroup()
{
    for (QGraphicsItem* item : m_list) {
        LaserPrimitive* primitive = qgraphicsitem_cast<LaserPrimitive*>(item);
        primitive->setJoinedGroup(false);
    }
    m_viewer->viewport()->repaint();
    m_joinedGroupAction->setEnabled(true);
    m_joinedUngroupAction->setEnabled(false);
}

ArrangeAlignCommand::ArrangeAlignCommand(LaserViewer * viewer, int type)
{
    m_viewer = viewer;
    m_scene = viewer->scene();
    m_group = m_viewer->group();
    m_type = type;
    m_alignTarget = LaserApplication::mainWindow->alignTarget();
    for (QGraphicsItem* item : m_group->childItems()) {
        LaserPrimitive* p = qgraphicsitem_cast<LaserPrimitive*>(item);
        m_undoMap.insert(p, p->sceneTransform());
    }
}

ArrangeAlignCommand::~ArrangeAlignCommand()
{
}

void ArrangeAlignCommand::undo()
{
    if (m_undoMap.isEmpty()) {
        return;
    }
    for (QGraphicsItem* item : m_group->childItems()) {
        LaserPrimitive* p = qgraphicsitem_cast<LaserPrimitive*>(item);
        m_group->removeFromGroup(p);
    }
    m_group->setTransform(QTransform());
    for (QMap<LaserPrimitive*, QTransform>::Iterator i = m_undoMap.begin(); i != m_undoMap.end(); i++) {
        i.key()->setTransform(i.value());
        i.key()->setPos(0, 0);
        m_scene->quadTreeNode()->upDatePrimitive(i.key());
        m_group->addToGroup(i.key());
    }
    emit m_viewer->selectedChangedFromMouse();
    m_viewer->viewport()->repaint();
}
void ArrangeAlignCommand::redo()
{
    if (!m_alignTarget) {
        return;
    }
    joinedGroupBoundsMap.clear();
    QPointF diff;
    QPointF c;
    for (QGraphicsItem* item : m_group->childItems()) {
        LaserPrimitive* p = qgraphicsitem_cast<LaserPrimitive*>(item);
       
        if (!m_alignTarget->isJoinedGroup()) {
            
            if (m_alignTarget != p) {
                QRect targetBounds = m_alignTarget->sceneBoundingRect();
                QRect scrBound;
                if (!p->isJoinedGroup()) {
                    scrBound = p->sceneBoundingRect();
                }
                else {
                    scrBound = joinedGroupSceneBounds(p);
                }
                if (!moveByType(p, targetBounds, scrBound)) {
                    return;
                }
            }
        }
        else {
            QList<LaserPrimitive*> alignTargetList = m_alignTarget->joinedGroupList();           
            if (!alignTargetList.contains(p)) {
                QRect targetBounds;
                utils::boundingRect(alignTargetList, targetBounds, QRect(), false);
                QRect scrBound;
                if (!p->isJoinedGroup()) {
                    scrBound = p->sceneBoundingRect();
                }
                else {
                    scrBound = joinedGroupSceneBounds(p);
                }
                if (!moveByType(p, targetBounds, scrBound)) {
                    return;
                }
            }
        }
    }   
    emit m_viewer->selectedChangedFromMouse();
    m_viewer->viewport()->repaint();
}

bool ArrangeAlignCommand::moveByType(LaserPrimitive* p, QRect target, QRect src)
{
    QPointF diff;
    switch (m_type) {
        case Qt::AlignCenter:{
            
            diff = m_group->mapFromScene(target.center()) - m_group->mapFromScene(src.center());
            break;
        }
        case Qt::AlignLeft: {
            diff = m_group->mapFromScene(target.left(), 0) - m_group->mapFromScene(src.left(), 0);
            break;
        }
        case Qt::AlignRight: {
            diff = m_group->mapFromScene(target.right(), 0) - m_group->mapFromScene(src.right(), 0);
            break;
        }
        case Qt::AlignHCenter: {
            diff = m_group->mapFromScene(target.center().x(), 0) - m_group->mapFromScene(src.center().x(), 0);
            break;
        }
        case Qt::AlignTop: {
            diff = m_group->mapFromScene(0, target.top()) - m_group->mapFromScene(0, src.top());
            break;
        }
        case Qt::AlignBottom: {
            diff = m_group->mapFromScene(0, target.bottom()) - m_group->mapFromScene(0, src.bottom());
            break;
        }
        case Qt::AlignVCenter: {
            diff = m_group->mapFromScene(0, target.center().y()) - m_group->mapFromScene(0, src.center().y());
            break;
        } 
    }
    p->moveBy(diff.x(), diff.y());
    qreal scaleVal = 1;
    switch (m_type) {
        case ArrangeType::AT_SameWidth: {
            qreal targetW = target.width();
            qreal srcW = src.width();
            if (targetW == 0) {
                scaleVal = 0.001/ src.width();
            }
            else if (srcW == 0) {
                scaleVal = 1;
            }
            else {
                scaleVal = targetW / srcW;
            }
            QTransform t1;
            t1.scale(scaleVal, 1);
            QTransform t2;
            QPointF diff = p->boundingRect().center() - t1.map(p->boundingRect().center());
            t2.translate(diff.x(), diff.y());
            QTransform t = p->transform() * t1 * t2;
            p->setTransform(t);
            break;
        }
        case ArrangeType::AT_SameHeight: {
            qreal targetH = target.height();
            qreal srcH = src.height();
            if (targetH == 0) {
                scaleVal = 1/srcH;
            }
            else if (srcH == 0) {
                scaleVal = 1;
            }
            else {
                scaleVal = targetH / srcH;
            }
            QTransform t1;
            t1.scale(1, scaleVal);
            QTransform t2;
            QPointF diff = p->boundingRect().center() - t1.map(p->boundingRect().center());
            t2.translate(diff.x(), diff.y());
            QTransform t = p->transform() * t1 * t2;
            p->setTransform(t);
            break;
        }
    }
    
    if (m_scene->maxRegion().contains(p->sceneBoundingRect())) {
        m_scene->quadTreeNode()->upDatePrimitive(p);
        return true;
    }
    else {
        QMessageBox::warning(m_viewer, ltr("WargingOverstepTitle"), ltr("WargingOverstepText"));
        undo();
        return false;
    }

}
//prevent repeate compute bounds
QRect ArrangeAlignCommand::joinedGroupSceneBounds(LaserPrimitive * p)
{
    QRect bounds;
    for (QMap<LaserPrimitive*, QRect>::Iterator i = joinedGroupBoundsMap.begin(); i != joinedGroupBoundsMap.end(); i++) {
        if (p->joinedGroupList().contains(i.key())) {
            return i.value();
        }
    }
    utils::boundingRect(p->joinedGroupList(), bounds, QRect(), false);
    joinedGroupBoundsMap.insert(p, bounds);
    return bounds;
}

ArrangeMoveToPageCommand::ArrangeMoveToPageCommand(LaserViewer * viewer, int type)
{
    m_viewer = viewer;
    m_type = type;
    m_scene = m_viewer->scene();
    m_group = m_viewer->group();
    QRect bounds;
    utils::boundingRect(m_group->childItems(), bounds, QRect(), false);
    m_undoPos = bounds.center();
}

ArrangeMoveToPageCommand::~ArrangeMoveToPageCommand()
{
}

void ArrangeMoveToPageCommand::undo()
{
    QRect bounds;
    utils::boundingRect(m_group->childItems(), bounds, QRect(), false);
    QPointF pos(bounds.center());
    m_group->moveBy(m_undoPos.x() - pos.x(), m_undoPos.y() - pos.y());
    for (QGraphicsItem* item : m_group->childItems()) {
        LaserPrimitive* p = qgraphicsitem_cast<LaserPrimitive*>(item);
        m_scene->quadTreeNode()->upDatePrimitive(p);
    }
    emit m_viewer->selectedChangedFromMouse();
    m_viewer->viewport()->repaint();
}

void ArrangeMoveToPageCommand::redo()
{
    //group
    QRect bounds;
    utils::boundingRect(m_group->childItems(), bounds, QRect(), false);
    QPointF pos;
    //scene
    QRect sceneBounds = m_scene->backgroundItem()->rect();
    QPointF scenePos;
    switch (m_type) {
        case Qt::TopLeftCorner: {
            pos = bounds.center();
            scenePos = sceneBounds.topLeft();
            break;
        }
        case Qt::TopRightCorner: {
            pos = bounds.center();
            scenePos = sceneBounds.topRight();
            break;
        }
        case Qt::BottomLeftCorner: {
            pos = bounds.center();
            scenePos = sceneBounds.bottomLeft();
            break;
        }
        case Qt::BottomRightCorner: {
            pos = bounds.center();
            scenePos = sceneBounds.bottomRight();
            break;
        }
        case AT_Center: {
            pos = bounds.center();
            scenePos = sceneBounds.center();
            break;
        }
        case AT_Left: {
            pos = QPointF(bounds.left(), bounds.center().y());
            scenePos = QPointF(sceneBounds.left(), sceneBounds.center().y());
            break;
        }
        case AT_Right: {
            pos = QPointF(bounds.right(), bounds.center().y());
            scenePos = QPointF(sceneBounds.right(), sceneBounds.center().y());
            break;
        }
        case AT_Top: {
            pos = QPointF(bounds.center().x(), bounds.top());
            scenePos = QPointF(sceneBounds.center().x(), sceneBounds.top());
            break;
        }
        case AT_Bottom: {
            pos = QPointF(bounds.center().x(), bounds.bottom());
            scenePos = QPointF(sceneBounds.center().x(), sceneBounds.bottom());
            break;
        }
    }
    m_group->moveBy(scenePos.x() - pos.x(), scenePos.y() - pos.y());
    if (m_scene->maxRegion().contains(m_group->sceneBoundingRect().toRect())) {
        for (QGraphicsItem* item : m_group->childItems()) {
            LaserPrimitive* p = qgraphicsitem_cast<LaserPrimitive*>(item);
            m_scene->quadTreeNode()->upDatePrimitive(p);
        }
        emit m_viewer->selectedChangedFromMouse();
        m_viewer->viewport()->repaint();
    }
    else {
        QMessageBox::warning(m_viewer, ltr("WargingOverstepTitle"), ltr("WargingOverstepText"));
        undo();
    }
    
    
}

CommonSelectionCommand::CommonSelectionCommand(LaserViewer* viewer, bool isInvert)
{
    m_viewer = viewer;
    m_group = m_viewer->group();
    for (QGraphicsItem* item : m_group->childItems()) {
        LaserPrimitive* p = qgraphicsitem_cast<LaserPrimitive*>(item);
        m_undoList.append(p);
    }
    m_isInvert = isInvert;
}

CommonSelectionCommand::~CommonSelectionCommand()
{
}

void CommonSelectionCommand::undo()
{
    m_group->removeAllFromGroup(true);
    m_group->setTransform(QTransform());
    for (LaserPrimitive* p : m_undoList) {
        m_group->addToGroup(p);
        p->setSelected(true);
    }
    m_viewer->viewport()->repaint();
}

void CommonSelectionCommand::redo()
{
    m_group->removeAllFromGroup();
    m_group->setTransform(QTransform());
    for (LaserPrimitive* p : m_viewer->scene()->document()->primitives()) {
        if (m_isInvert) {
            if (p->isSelected()) {
                p->setSelected(false);
            }
            else {
                p->setSelected(true);
                m_group->addToGroup(p);
            }
        }
        //select all
        else {
            p->setSelected(true);
            m_group->addToGroup(p);
        }
        
    }
    if (!m_viewer->group()->isEmpty() && StateControllerInst.isInState(StateControllerInst.documentIdleState())) {
        emit m_viewer->idleToSelected();
    }
    m_viewer->viewport()->repaint();
}

DistributeUndoCommand::DistributeUndoCommand(LaserViewer * viewer, int type)
{
    m_viewer = viewer;
    m_group = m_viewer->group();
    m_type = type;
    m_scene = m_viewer->scene();
    m_frontestPrimitive = nullptr;
    m_backestPrimitive = nullptr;
}

DistributeUndoCommand::~DistributeUndoCommand()
{
}

void DistributeUndoCommand::findTopAndBottomPrimitive(LaserPrimitive* & topPrimitive, LaserPrimitive* & bottomPrimitive)
{
    topPrimitive = nullptr;
    bottomPrimitive = nullptr;
    int index = 0;
    for (QGraphicsItem* item : m_group->childItems()) {
        
        LaserPrimitive* p = qgraphicsitem_cast<LaserPrimitive*>(item);
        QRectF bounds = p->sceneBoundingRect();
        if (index == 0) {
            topPrimitive = p;
            bottomPrimitive = p;
        }
        else {
            switch (m_type)
            {
                case ArrangeType::AT_VCentered:
                {
                    if (topPrimitive->sceneBoundingRect().center().y() > bounds.center().y()) {
                        topPrimitive = p;
                    }
                    if (bottomPrimitive->sceneBoundingRect().center().y() < bounds.center().y()) {
                        bottomPrimitive = p;
                    }
                    break;
                }
                case ArrangeType::AT_VSpaced:
                {
                    if (topPrimitive->sceneBoundingRect().top() > bounds.top()) {
                        topPrimitive = p;
                    }
                    if (bottomPrimitive->sceneBoundingRect().bottom() < bounds.bottom()) {
                        bottomPrimitive = p;
                    }
                    break;
                }
                case ArrangeType::AT_HCentered:
                {
                    if (topPrimitive->sceneBoundingRect().center().x() > bounds.center().x()) {
                        topPrimitive = p;
                    }
                    if (bottomPrimitive->sceneBoundingRect().center().x() < bounds.center().x()) {
                        bottomPrimitive = p;
                    }
                    break;
                }
                case ArrangeType::AT_HSpaced:
                {
                    if (topPrimitive->sceneBoundingRect().left() > bounds.left()) {
                        topPrimitive = p;
                    }
                    if (bottomPrimitive->sceneBoundingRect().right() < bounds.right()) {
                        bottomPrimitive = p;
                    }
                    break;
                }
            }
            
        }
        index++;
    }
    
}

void DistributeUndoCommand::undo()
{
    for (QGraphicsItem* item : m_group->childItems()) {
        LaserPrimitive* p = qgraphicsitem_cast<LaserPrimitive*>(item);
        m_group->removeFromGroup(p);
    }
    m_group->setTransform(QTransform());
    for (QMap<LaserPrimitive*, QTransform>::Iterator i = m_undoMap.begin(); i != m_undoMap.end(); i++) {
        i.key()->setTransform(i.value());
        i.key()->setPos(0, 0);
        m_scene->quadTreeNode()->upDatePrimitive(i.key());
        m_group->addToGroup(i.key());
    }
    m_group->addToGroup(m_frontestPrimitive);
    m_group->addToGroup(m_backestPrimitive);
    emit m_viewer->selectedChangedFromMouse();
    m_viewer->viewport()->repaint();
    
}

void DistributeUndoCommand::redo()
{
    
    findTopAndBottomPrimitive(m_frontestPrimitive, m_backestPrimitive);
    //except toppestPrimitive and bottommerPrimitive
    qreal othersLength = 0;
    QList<LaserPrimitive*> sortedList;
    //sort primitives except toppestPrimitive and bottommerPrimitive
    int whileIndex = 0;
    while (1) {
        int index = 0;
        LaserPrimitive* topperPrimitive = nullptr;
        for (QGraphicsItem* item : m_group->childItems()) {

            LaserPrimitive* p = qgraphicsitem_cast<LaserPrimitive*>(item);
            if (whileIndex == 0 && p != m_frontestPrimitive && p != m_backestPrimitive) {
                switch (m_type)
                {
                    case ArrangeType::AT_VCentered:
                    {
                        othersLength = 0;
                        break;
                    }
                    case ArrangeType::AT_VSpaced: 
                    {
                        othersLength += p->sceneBoundingRect().height();
                        break;
                    }
                    case ArrangeType::AT_HCentered:
                    {
                        othersLength = 0;
                        break;
                    }
                    case ArrangeType::AT_HSpaced:
                    {
                        othersLength += p->sceneBoundingRect().width();
                        break;
                    }
                }
                
            }
            if (p == m_frontestPrimitive || p == m_backestPrimitive
                || sortedList.contains(p)) {
                continue;
            }
            if (index == 0) {
                topperPrimitive = p;
            }
            if (topperPrimitive->sceneBoundingRect().top() > p->sceneBoundingRect().top()) {

                topperPrimitive = p;
            }
            index++;
        }
        if (topperPrimitive) {
            sortedList.append(topperPrimitive);
        }
        else {
            break;
        }
        whileIndex++;
    }
    qreal minVal;
    qreal maxVal;
    switch (m_type) {
        case ArrangeType::AT_VSpaced: {
            minVal = m_frontestPrimitive->sceneBoundingRect().bottom();
            maxVal = m_backestPrimitive->sceneBoundingRect().top();
            
            break;
        }
        case ArrangeType::AT_VCentered: {
            minVal = m_frontestPrimitive->sceneBoundingRect().center().y();
            maxVal = m_backestPrimitive->sceneBoundingRect().center().y();
            break;
        }
        case ArrangeType::AT_HSpaced: {
            minVal = m_frontestPrimitive->sceneBoundingRect().right();
            maxVal = m_backestPrimitive->sceneBoundingRect().left();
            break;
        }
        case ArrangeType::AT_HCentered: {
            minVal = m_frontestPrimitive->sceneBoundingRect().center().x();
            maxVal = m_backestPrimitive->sceneBoundingRect().center().x();
            break;
        }
    }
    qreal diff = (maxVal - minVal) - othersLength;
    qreal space = diff / (sortedList.size() + 1);
    qreal baseVal = minVal;
    
    for (LaserPrimitive* primitive : sortedList) {
        QPointF pos;
        m_undoMap.insert(primitive, primitive->sceneTransform());
        switch (m_type)
        {
            case ArrangeType::AT_VCentered: {
                pos = primitive->scenePos() + QPointF(0,
                    baseVal - primitive->sceneBoundingRect().center().y() + space);
                primitive->setPos(m_group->mapFromScene(pos));
                baseVal = primitive->sceneBoundingRect().center().y();
                break;
            }
            case ArrangeType::AT_VSpaced:
            {
                pos = primitive->scenePos() + QPointF(0,
                    baseVal - primitive->sceneBoundingRect().top() + space);
                
                primitive->setPos(m_group->mapFromScene(pos));
                baseVal = primitive->sceneBoundingRect().bottom();
                break;
            }
            case ArrangeType::AT_HCentered: {
                pos = primitive->scenePos() + QPointF(
                    baseVal - primitive->sceneBoundingRect().center().x() + space,
                    0);
                primitive->setPos(m_group->mapFromScene(pos));
                baseVal = primitive->sceneBoundingRect().center().x();
                break;
            }
            case ArrangeType::AT_HSpaced:
            {
                pos = primitive->scenePos() + QPointF(
                    baseVal - primitive->sceneBoundingRect().left() + space,
                    0);
                primitive->setPos(m_group->mapFromScene(pos));
                baseVal = primitive->sceneBoundingRect().right();
                break;
            }
            
        } 
        m_scene->quadTreeNode()->upDatePrimitive(primitive);
    }
    //bottommest primitive need move
    if (diff < 0 && (m_type == AT_VSpaced || m_type == AT_HSpaced)) {
        QPointF pos;
        switch (m_type)
        {
        /*case ArrangeType::AT_VCentered: {
            pos = m_backestPrimitive->scenePos() + QPointF(0,
                baseVal - m_backestPrimitive->sceneBoundingRect().center().y() + space);
        }*/
        case ArrangeType::AT_VSpaced:
        {
            pos = m_backestPrimitive->scenePos() + QPointF(0,
                baseVal - m_backestPrimitive->sceneBoundingRect().top() + space);
            break;
        }
        /*case ArrangeType::AT_HCentered: {
            pos = m_backestPrimitive->scenePos() + QPointF(
                baseVal - m_backestPrimitive->sceneBoundingRect().center().x() + space,
                0);
            break;
        }*/
        case ArrangeType::AT_HSpaced:
        {
            pos = m_backestPrimitive->scenePos() + QPointF(
                baseVal - m_backestPrimitive->sceneBoundingRect().left() + space,
                0);
            break;
        }
        }
        m_undoMap.insert(m_backestPrimitive, m_backestPrimitive->sceneTransform());
        m_backestPrimitive->setPos(m_group->mapFromScene(pos));
        m_scene->quadTreeNode()->upDatePrimitive(m_backestPrimitive);
    }
    //is extend max region
    if (m_scene->maxRegion().contains(m_group->sceneBoundingRect().toRect())) {      
        emit m_viewer->selectedChangedFromMouse();
        m_viewer->viewport()->repaint();
    }
    else {
        QMessageBox::warning(m_viewer, ltr("WargingOverstepTitle"), ltr("WargingOverstepText"));
        undo();
    }
}
