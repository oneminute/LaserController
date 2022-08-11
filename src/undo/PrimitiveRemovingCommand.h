#pragma once

#include "BaseUndoCommand.h"

class PrimitiveRemovingCommand : public BaseUndoCommand
{
public:
    explicit PrimitiveRemovingCommand(
        const QString& text,
        QObject* target,
        QUndoCommand* parent
    );
    ~PrimitiveRemovingCommand();

    virtual void undo() override;
    virtual void redo() override;

private:
};