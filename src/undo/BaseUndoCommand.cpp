#include "BaseUndoCommand.h"

BaseUndoCommand::BaseUndoCommand(
    const QString& text, 
    LaserViewer* viewer,
    LaserScene* scene,
    LaserDocument* document,
    QUndoCommand* parent)
    : QUndoCommand(text, parent)
    , m_viewer(viewer)
    , m_scene(scene)
    , m_document(document)
{
}

BaseUndoCommand::~BaseUndoCommand() {}

void BaseUndoCommand::callUndoCallback()
{
    if (m_undoCallback)
        m_undoCallback();
}

void BaseUndoCommand::callRedoCallback()
{
    if (m_redoCallback)
        m_redoCallback();
}

