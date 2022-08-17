#include "PrimitiveRemovingCommand.h"

#include "primitive/LaserPrimitive.h"
#include "scene/LaserDocument.h"
#include "scene/LaserLayer.h"
#include "widget/LaserViewer.h"

PrimitiveRemovingCommand::PrimitiveRemovingCommand(
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
    , m_cloned(nullptr)
    , m_added(nullptr)
{
    m_cloned = target->clone();
}

PrimitiveRemovingCommand::~PrimitiveRemovingCommand()
{
    m_cloned->deleteLater();
}

void PrimitiveRemovingCommand::undo()
{
    LaserLayer* layer = document()->layerById(m_layerId);
    if (!layer)
    {
        qLogW << "undo removing primitive failure: layer does not exist.";
        return;
    }
    m_added = m_cloned->clone();
    document()->addPrimitive(m_added, layer);
    callUndoCallback();
}

void PrimitiveRemovingCommand::redo()
{
    LaserLayer* layer = document()->layerById(m_layerId);
    if (!layer)
    {
        qLogW << "redo removing primitive failure: layer does not exist.";
        return;
    }
    LaserPrimitive* primitive = layer->primitiveById(m_primitiveId);
    if (!primitive)
    {
        qLogW << "redo removing primitive failure: primitive " << m_primitiveId << " does not exist.";
        return;
    }
    document()->removePrimitive(primitive, false, true, true);
    m_added = nullptr;
    emit viewer()->endEditing();
    callRedoCallback();
}

