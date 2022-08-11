#include "PrimitiveRemovingCommand.h"

PrimitiveRemovingCommand::PrimitiveRemovingCommand(
    const QString& text, 
    QObject* target, 
    QUndoCommand* parent)
    : BaseUndoCommand(text, parent)
{
}

PrimitiveRemovingCommand::~PrimitiveRemovingCommand()
{
}

void PrimitiveRemovingCommand::undo()
{
}

void PrimitiveRemovingCommand::redo()
{
}

