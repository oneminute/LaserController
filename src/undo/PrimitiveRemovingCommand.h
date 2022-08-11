#pragma once

#include "BaseUndoCommand.h"

class LaserPrimitive;

class PrimitiveRemovingCommand : public BaseUndoCommand
{
public:
    explicit PrimitiveRemovingCommand(
        const QString& text,
        LaserViewer* viewer,
        LaserScene* scene,
        LaserDocument* doc,
        const QString& primitiveId,
        const QString& layerId,
        LaserPrimitive* target,
        QUndoCommand* parent
    );
    ~PrimitiveRemovingCommand();

    virtual void undo() override;
    virtual void redo() override;

private:
    QString m_primitiveId;
    QString m_layerId;
    LaserPrimitive* m_cloned;
    LaserPrimitive* m_added;
};