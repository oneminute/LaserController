#pragma once

#include <QUndoCommand>

class LaserDocument;

class BaseUndoCommand : public QUndoCommand
{
public:
    explicit BaseUndoCommand(
        const QString& text, 
        QUndoCommand* parent = nullptr);
    
    ~BaseUndoCommand();

private:
};