#pragma once

#include "common/common.h"
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
        LaserPrimitiveType type,
        QUndoCommand* parent = nullptr);
    explicit PrimitiveAddingCommand(
        const QString& text,
        LaserViewer* viewer,
        LaserScene* scene,
        LaserDocument* doc,
        LaserPrimitive* primitive,
        QUndoCommand* parent = nullptr);
    ~PrimitiveAddingCommand();

    virtual void undo() override;
    virtual void redo() override;

    LaserPrimitive* primitive() const;

private:
    LaserPrimitiveType m_primitiveType;
    QString m_primitiveId;
    QString m_layerId;
    LaserPrimitive* m_primitive;
};