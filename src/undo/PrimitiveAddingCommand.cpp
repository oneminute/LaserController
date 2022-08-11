#include "PrimitiveAddingCommand.h"

#include "primitive/LaserPrimitiveHeaders.h"
#include "scene/LaserDocument.h"
#include "scene/LaserLayer.h"

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
    , m_cloned(target)
    , m_added(nullptr)
{
    //m_cloned = target->clone();
}

PrimitiveAddingCommand::~PrimitiveAddingCommand()
{
    delete m_cloned;
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
    m_added = nullptr;
}

void PrimitiveAddingCommand::redo()
{
    LaserLayer* layer = document()->layerById(m_layerId);
    if (!layer)
    {
        qLogW << "undo adding primitive failure: layer does not exist.";
        return;
    }
    m_added = m_cloned->clone();
    document()->addPrimitive(m_added, layer);
}

LaserPrimitive* PrimitiveAddingCommand::added() const
{
    return m_added;
}

