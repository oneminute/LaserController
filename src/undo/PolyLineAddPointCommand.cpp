#include "PolyLineAddPointCommand.h"

#include "primitive/LaserPolyLine.h"

PolyLineAddPointCommand::PolyLineAddPointCommand(const QString& text, 
    QObject* target, 
    const QPoint& point,
    int pointIndex, 
    QUndoCommand* parent)
    : BaseUndoCommand(text, target, parent)
    , m_point(point)
    , m_pointIndex(pointIndex)
{
}

PolyLineAddPointCommand::~PolyLineAddPointCommand()
{
}

void PolyLineAddPointCommand::undo()
{
    LaserPolyline* polyline = qobject_cast<LaserPolyline*>(target());
    if (polyline == nullptr)
        return;

    polyline->removePoint(m_pointIndex);
}

void PolyLineAddPointCommand::redo()
{
    LaserPolyline* polyline = qobject_cast<LaserPolyline*>(target());
    if (polyline == nullptr)
        return;

    polyline->appendPoint(m_point);
}
