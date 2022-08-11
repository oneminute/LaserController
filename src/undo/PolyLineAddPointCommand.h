#pragma once

#include <QPoint>

#include "BaseUndoCommand.h"
#include "primitive/LaserPrimitiveDeclaration.h"

class PolylineAddPointCommand : public BaseUndoCommand
{
public:
    explicit PolylineAddPointCommand(
        const QString& text, 
        LaserDocument* document,
        const QString& primitiveId,
        const QPoint& point,
        int pointIndex,
        QUndoCommand* parent = nullptr);
    ~PolylineAddPointCommand();

    virtual void undo() override;
    virtual void redo() override;

private:
    LaserDocument* m_doc;
    QString m_primitiveId;
    QPoint m_point;
    int m_pointIndex;
};