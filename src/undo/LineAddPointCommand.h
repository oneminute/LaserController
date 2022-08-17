#pragma once

#include <QPoint>

#include "BaseUndoCommand.h"
#include "primitive/LaserPrimitiveDeclaration.h"

class LineAddPointCommand : public BaseUndoCommand
{
public:
    explicit LineAddPointCommand(
        const QString& text, 
        LaserViewer* viewer,
        LaserScene* scene,
        LaserDocument* document,
        const QString& primitiveId,
        const QPoint& point,
        int pointIndex,
        QUndoCommand* parent = nullptr);
    ~LineAddPointCommand();

    virtual void undo() override;
    virtual void redo() override;

private:
    QString m_primitiveId;
    QPoint m_point;
    int m_pointIndex;
};
