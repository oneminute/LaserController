#pragma once

#include <QUndoCommand>

class BaseUndoCommand : public QUndoCommand
{
public:
    explicit BaseUndoCommand(const QString& text, QObject* target, QUndoCommand* parent = nullptr)
        : QUndoCommand(text, parent)
        , m_target(target)
    {
        Q_ASSERT(target);
    }
    
    ~BaseUndoCommand() {}

    QObject* target() { return m_target; }

private:
    QObject* m_target;
};