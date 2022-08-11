#pragma once

#include "BaseUndoCommand.h"

class PrimitiveRemovingCommand : public BaseUndoCommand
{
public:
    explicit PrimitiveRemovingCommand(
        const QString& text,
        LaserViewer* viewer,
        LaserScene* scene,
        LaserDocument* doc,
        QObject* target,
        QUndoCommand* parent
    );
    ~PrimitiveRemovingCommand();

    virtual void undo() override;
    virtual void redo() override;

private:
};