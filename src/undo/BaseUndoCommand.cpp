#include "BaseUndoCommand.h"

BaseUndoCommand::BaseUndoCommand(
    const QString& text, 
    QUndoCommand* parent)
    : QUndoCommand(text, parent)
{
}

BaseUndoCommand::~BaseUndoCommand() {}

