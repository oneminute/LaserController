#pragma once

#include <QUndoCommand>

class LaserDocument;
class LaserScene;
class LaserViewer;

class BaseUndoCommand : public QUndoCommand
{
public:
    explicit BaseUndoCommand(
        const QString& text, 
        LaserViewer* viewer,
        LaserScene* scene,
        LaserDocument* document,
        QUndoCommand* parent = nullptr);
    
    ~BaseUndoCommand();

    LaserViewer* viewer() const { return m_viewer; }
    LaserScene* scene() const { return m_scene; }
    LaserDocument* document() const { return m_document; }

private:
    LaserViewer* m_viewer;
    LaserScene* m_scene;
    LaserDocument* m_document;
};