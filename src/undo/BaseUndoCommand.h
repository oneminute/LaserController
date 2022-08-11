#pragma once

#include <QUndoCommand>

class LaserDocument;
class LaserScene;
class LaserViewer;

class BaseUndoCommand : public QUndoCommand
{
public:
    typedef std::function<void()> Callback;

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

    void callUndoCallback();
    void callRedoCallback();

    void setUndoCallback(Callback fn) { m_undoCallback = fn; }
    Callback undoCallback() { return m_undoCallback; }
    void setRedoCallback(Callback fn) { m_redoCallback = fn; }
    Callback redoCallback() { return m_redoCallback; }

private:
    LaserViewer* m_viewer;
    LaserScene* m_scene;
    LaserDocument* m_document;
    Callback m_undoCallback;
    Callback m_redoCallback;
};