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

PrimitiveAddingCommand::PrimitiveAddingCommand(
    const QString& text,
    LaserViewer* viewer,
    LaserScene* scene,
    LaserDocument* doc,
    LaserPrimitive* primitive,
    QUndoCommand* parent)
    : BaseUndoCommand(text, viewer, scene, doc, parent)
    , m_primitiveType(primitive->primitiveType())
    , m_primitive(primitive)
{
}

PrimitiveAddingCommand::~PrimitiveAddingCommand()
{
    //m_primitive->deleteLater();
    //m_primitive = nullptr;
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
    viewer()->viewport()->update();
}

void PrimitiveAddingCommand::redo()
{
    if (!m_primitive)
    {
        m_primitive = LaserPrimitive::createPrimitive(m_primitiveType, document());
        if (m_primitiveId.isEmpty())
            m_primitiveId = m_primitive->id();
        else
            m_primitive->setId(m_primitiveId);
        }
    if (!m_primitive)
    {
        qLogW << "redo adding primitive failure: can not create primitive of type "
            << m_primitiveType;
        return;
    }
    LaserLayer* layer = m_primitive->layer();
    document()->addPrimitive(m_primitive, layer);
    m_layerId = layer->id();
    callRedoCallback();
}

LaserPrimitive* PrimitiveAddingCommand::primitive() const
{
    return m_primitive;
}

