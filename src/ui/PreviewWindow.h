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

    qreal progress() const;
    void intendUpdate();

public slots:
    void updatePreviewArea();
    void reset();
    void resetProgress();

    void addPath(const QPainterPath& path, QPen pen = QPen(Qt::blue, 1, Qt::SolidLine), const QString& label = QString());
    void addLine(const QLineF& line, QPen pen = QPen(Qt::black, 1, Qt::SolidLine), const QString& label = QString());
    void addPoints(const QList<QPointF>& points, QPen pen = QPen(Qt::gray, 1, Qt::SolidLine), int style = 0);
    void addPoint(const QPointF& point, QPen pen = QPen(Qt::gray, 0.5, Qt::SolidLine), int style = 0);
    void setTitle(const QString& msg);
    void registerProgressCode(quint32 code, qreal quota);
    void registerProgressCode(void* ptr, qreal quota);
    void addProgress(quint32 code, qreal deltaProgress, const QString& msg = QString(), bool ignoreQuota = false);
    void addProgress(void* ptr, qreal deltaProgress, const QString& msg = QString(), bool ignoreQuota = false);
    void setProgress(qreal progress);
    void setProgress(quint32 code, qreal progress);
    void setProgress(void* code, qreal progress);
    void addMessage(const QString& message);

    void showAsync();

protected slots:
    void onAddPath(const QPainterPath& path, QPen pen = QPen(Qt::blue, 1, Qt::SolidLine), const QString& label = QString());
    void onAddLine(const QLineF& line, QPen pen = QPen(Qt::black, 1, Qt::SolidLine), const QString& label = QString());
    void onAddPoints(const QList<QPointF>& points, QPen pen = QPen(Qt::gray, 0.5, Qt::SolidLine), int style = 0);
    void onAddPoint(const QPointF& point, QPen pen = QPen(Qt::gray, 0.5, Qt::SolidLine), int style = 0);
    void onSetTitle(const QString& msg);
    void onAddProgress(quint32 code, qreal deltaProgress, const QString& msg = QString(), bool ignoreQuota = false);
    void onSetProgress(qreal progress);
    void onSetProgress(quint32 code, qreal progress);
    void onAddMessage(const QString& message);

    void onShowAsync();

signals:
    void addPathSignal(const QPainterPath& path, QPen pen = QPen(Qt::blue, 1, Qt::SolidLine), const QString& label = QString());
    void addLineSignal(const QLineF& line, QPen pen = QPen(Qt::black, 1, Qt::SolidLine), const QString& label = QString());
    void addPointsSignal(const QList<QPointF>& points, QPen pen = QPen(Qt::gray, 0.5, Qt::SolidLine), int style = 0);
    void addPointSignal(const QPointF& point, QPen pen = QPen(Qt::gray, 0.5, Qt::SolidLine), int style = 0);
    void setTitleSignal(const QString& msg);
    void addProgressSignal(quint32 code, qreal deltaProgress, const QString& msg = QString(), bool ignoreQuota = false);
    void setProgressSignal(qreal progress);
    void setProgressSignal(quint32 code, qreal progress);
    void addMessageSignal(const QString& message);

    void progressUpdated(qreal progress);

    void showAsyncSignal();

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
    QAction* m_actionZoomIn;
    QAction* m_actionZoomOut;
    QAction* m_actionNextStep;

    qreal m_progress;
    QMap<quint32, qreal> m_progressQuotas;
    qreal m_sumQuotas;

    QTimer m_updateTimer;
};

#endif // PREVIEWWINDOW_H
