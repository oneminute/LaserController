#ifndef PREVIEWWINDOW_H
#define PREVIEWWINDOW_H

#include <QAction>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <DockManager.h>

class PreviewViewer;

class PreviewWindow : public QMainWindow
{
public:
    explicit PreviewWindow(QWidget* parent = nullptr);
    ~PreviewWindow();

    int exec();

    int result() const;

public slots:
    void addMessage(const QString& msg);
    void setTitle(const QString& msg);
    void setProgress(float progress);
    void finished();

private:
    ads::CDockManager* m_dockManager;
    ads::CDockWidget* m_logDockWidget;
    PreviewViewer* m_viewer;
    QPlainTextEdit* m_textEditLog;
    QProgressBar* m_progressBar;

    // actions
    QAction* m_actionZoomIn;
    QAction* m_actionZoomOut;
    QAction* m_actionNextStep;

};

#endif // PREVIEWWINDOW_H
