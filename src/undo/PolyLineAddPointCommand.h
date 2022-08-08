#pragma once

#include <QPoint>

#include "BaseUndoCommand.h"

class PolyLineAddPointCommand : public BaseUndoCommand
{
public:
    explicit PolyLineAddPointCommand(const QString& text, 
        QObject* target, 
        const QPoint& point,
        int pointIndex, 
        QUndoCommand* parent = nullptr);
    ~PolyLineAddPointCommand();

    virtual void undo() override;
    virtual void redo() override;

private:
    QPoint m_point;
    int m_pointIndex;
};