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
    LaserPrimitiveType type,
    QUndoCommand* parent)
    : BaseUndoCommand(text, viewer, scene, doc, parent)
    , m_primitiveType(type)
    , m_primitive(nullptr)
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
    m_primitive = nullptr;
    callUndoCallback();
}

void PrimitiveAddingCommand::redo()
{
    m_primitive = LaserPrimitive::createPrimitive(m_primitiveType, document());
    if (!m_primitive)
    {
        qLogW << "undo adding primitive failure: can not create primitive of type "
            << m_primitiveType;
        return;
    }
    LaserLayer* layer = m_primitive->layer();
    document()->addPrimitive(m_primitive, layer);
    callRedoCallback();
}

LaserPrimitive* PrimitiveAddingCommand::primitive() const
{
    return m_primitive;
}

