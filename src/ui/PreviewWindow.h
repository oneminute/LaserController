#ifndef PREVIEWWINDOW_H
#define PREVIEWWINDOW_H

#include <QAction>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <DockManager.h>

class LaserPrimitive;
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

    void addPrimitive(LaserPrimitive* primitive);
    void addPath(const QPainterPath& path, QPen pen = QPen(Qt::blue, 1, Qt::SolidLine), const QString& label = QString());
    void addLine(const QLineF& line, QPen pen = QPen(Qt::black, 1, Qt::SolidLine), const QString& label = QString());

    void updatePreviewArea();

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
