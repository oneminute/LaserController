#ifndef PREVIEWWINDOW_H
#define PREVIEWWINDOW_H

#include <QAction>
#include <QDialog>
#include <QMainWindow>
#include <DockManager.h>
#include <QTimer>
#include <QPen>

class LaserPrimitive;
class PreviewViewer;
class PreviewScene;
class QTreeView;
class QProgressBar;
class QPlainTextEdit;

class PreviewWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit PreviewWindow(QWidget* parent = nullptr);
    ~PreviewWindow();

    void intendUpdate();

public slots:
    void updatePreviewArea();
    void reset();

    void addPath(const QPainterPath& path, QPen pen = QPen(Qt::blue, 1, Qt::SolidLine), const QString& label = QString());
    void addLine(const QLineF& line, QPen pen = QPen(Qt::black, 1, Qt::SolidLine), const QString& label = QString());
    void setTitle(const QString& msg);
    void addMessage(const QString& message);

protected slots:
    void onAddPath(const QPainterPath& path, QPen pen = QPen(Qt::blue, 1, Qt::SolidLine), const QString& label = QString());
    void onAddLine(const QLineF& line, QPen pen = QPen(Qt::black, 1, Qt::SolidLine), const QString& label = QString());
    void onSetTitle(const QString& msg);
    void onAddMessage(const QString& message);

signals:
    void addPathSignal(const QPainterPath& path, QPen pen = QPen(Qt::blue, 1, Qt::SolidLine), const QString& label = QString());
    void addLineSignal(const QLineF& line, QPen pen = QPen(Qt::black, 1, Qt::SolidLine), const QString& label = QString());
    void setTitleSignal(const QString& msg);
    void addMessageSignal(const QString& message);

    void progressUpdated(qreal progress);

private:
    ads::CDockManager* m_dockManager;
    ads::CDockWidget* m_canvasDockWidget;
    ads::CDockWidget* m_logDockWidget;
    ads::CDockWidget* m_progressDockWidget;
    PreviewViewer* m_viewer;
    PreviewScene* m_scene;
    QPlainTextEdit* m_textEditLog;
    QProgressBar* m_progressBar;
    QTreeView* m_treeViewProgress;

    // actions
    //QAction* m_actionZoomIn;
    //QAction* m_actionZoomOut;
    //QAction* m_actionNextStep;

    QTimer m_updateTimer;
};

#endif // PREVIEWWINDOW_H
