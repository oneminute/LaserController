#include "PrimitiveRemovingCommand.h"

PrimitiveRemovingCommand::PrimitiveRemovingCommand(
    const QString& text, 
    LaserViewer* viewer,
    LaserScene* scene,
    LaserDocument* doc,
    QObject* target, 
    QUndoCommand* parent)
    : BaseUndoCommand(text, viewer, scene, doc, parent)
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

