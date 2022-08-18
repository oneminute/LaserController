#include "PrimitiveAddingCommand.h"

#include "primitive/LaserPrimitiveHeaders.h"
#include "scene/LaserDocument.h"
#include "scene/LaserLayer.h"
#include "scene/LaserScene.h"
#include "widget/LaserViewer.h"

PrimitiveAddingCommand::PrimitiveAddingCommand(
    const QString& text, 
    LaserViewer* viewer,
    LaserScene* scene,
    LaserDocument* doc,
    const QString& primitiveId,
    const QString& layerId,
    LaserPrimitive* target, 
    QUndoCommand* parent)
    : BaseUndoCommand(text, viewer, scene, doc, parent)
    , m_primitiveId(primitiveId)
    , m_layerId(layerId)
    , m_origin(target)
    , m_cloned(nullptr)
{
}

PrimitiveAddingCommand::~PrimitiveAddingCommand()
{
}

void PrimitiveAddingCommand::undo()
{
    LaserLayer* layer = document()->layerById(m_layerId);
    if (!layer)
    {
        qLogW << "undo adding primitive failure: layer does not exist.";
        return;
    }
    LaserPrimitive* primitive = layer->primitiveById(m_primitiveId);
    if (!primitive)
    {
        qLogW << "undo adding primitive failure: primitive does not exist.";
        return;
    }
    document()->removePrimitive(primitive, false, true, true);
    m_cloned = nullptr;
    callUndoCallback();
}

void PrimitiveAddingCommand::redo()
{
    LaserLayer* layer = document()->layerById(m_layerId);
    if (!layer)
    {
        qLogW << "undo adding primitive failure: layer does not exist.";
        return;
    }
    m_cloned = m_origin->clone();
    document()->addPrimitive(m_cloned, layer);
    callRedoCallback();
}

LaserPrimitive* PrimitiveAddingCommand::cloned() const
{
    return m_cloned;
}

