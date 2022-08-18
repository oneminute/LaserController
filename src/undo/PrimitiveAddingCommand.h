#pragma once

#include "BaseUndoCommand.h"
#include "primitive/LaserPrimitiveDeclaration.h"

class LaserDocument;

class PrimitiveAddingCommand : public BaseUndoCommand
{
public:
    explicit PrimitiveAddingCommand(
        const QString& text,
        LaserViewer* viewer,
        LaserScene* scene,
        LaserDocument* doc,
        const QString& primitiveId,
        const QString& layerId,
        LaserPrimitive* target, 
        QUndoCommand* parent = nullptr);
    ~PrimitiveAddingCommand();

    virtual void undo() override;
    virtual void redo() override;

    LaserPrimitive* cloned() const;

private:
    QString m_primitiveId;
    QString m_layerId;
    LaserPrimitive* m_origin;
    LaserPrimitive* m_cloned;
};