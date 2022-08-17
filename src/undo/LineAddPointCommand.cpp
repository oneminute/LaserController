#include "LineAddPointCommand.h"

#include "primitive/LaserLine.h"
#include "scene/LaserDocument.h"
#include "state/StateController.h"
#include "widget/LaserViewer.h"

LineAddPointCommand::LineAddPointCommand(
    const QString& text, 
    LaserViewer* viewer,
    LaserScene* scene,
    LaserDocument* document,
    const QString& primitiveId,
    const QPoint& point,
    int pointIndex,
    QUndoCommand* parent)
    : BaseUndoCommand(text, viewer, scene, document, parent)
    , m_primitiveId(primitiveId)
    , m_point(point)
    , m_pointIndex(pointIndex)
{
}

LineAddPointCommand::~LineAddPointCommand()
{
}

void LineAddPointCommand::undo()
{
    LaserLine* target = qgraphicsitem_cast<LaserLine*>(document()->primitiveById(m_primitiveId));
    if (!target)
    {
        qLogW << "Undo adding line point failure because of there's no primitive with id " << m_primitiveId;
        return;
    }
    target->setEditing(true);
    target->removePoint(m_pointIndex);
    if (!StateControllerInst.isInState(StateControllerInst.documentPrimitiveState()))
    {
        viewer()->setEditingPrimitiveId(m_primitiveId);
        emit viewer()->beginEditing();
    }
}

void LineAddPointCommand::redo()
{
    LaserLine* target = qgraphicsitem_cast<LaserLine*>(document()->primitiveById(m_primitiveId));
    if (!target)
    {
        qLogW << "Undo adding line point failure because of there's no primitive with id " << m_primitiveId;
        return;
    }
    target->setEditing(true);
    if (!StateControllerInst.isInState(StateControllerInst.documentPrimitiveState()))
    {
        viewer()->setEditingPrimitiveId(m_primitiveId);
        emit viewer()->beginEditing();
    }
    target->appendPoint(m_point);
}