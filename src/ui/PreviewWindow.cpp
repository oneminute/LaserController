#include "PreviewWindow.h"

#include "common/common.h"
#include "scene/LaserPrimitive.h"
#include <LaserApplication.h>
#include <laser/LaserDevice.h>
#include <widget/PreviewScene.h>
#include <QDesktopWidget>
#include <QEventLoop>
#include <QMenuBar>
#include <QPen>
#include <QPointer>
#include <QToolBar>
#include <QtMath>
#include <QStatusBar>
#include <DockAreaTabBar.h>
#include <DockAreaTitleBar.h>
#include <DockAreaWidget.h>
#include <DockComponentsFactory.h>
#include "widget/PreviewViewer.h"

using namespace ads;

PreviewWindow::PreviewWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_progress(0.0)
{
    // menu bar
    QMenuBar* menuBar = new QMenuBar;
    setMenuBar(menuBar);

    // tool bar
    QToolBar* toolBarProcedure = new QToolBar;
    addToolBar(toolBarProcedure);

    // status bar
    QStatusBar* statusBar = new QStatusBar;
    setStatusBar(statusBar);

    // actions
    m_actionZoomIn = new QAction(tr("Zoom In"));
    toolBarProcedure->addAction(m_actionZoomIn);
   

    m_actionZoomOut = new QAction(tr("Zoom Out"));
    toolBarProcedure->addAction(m_actionZoomOut);

    m_actionNextStep = new QAction(tr("Next"));
    toolBarProcedure->addAction(m_actionNextStep);

    // preview viewer widget
    m_viewer = new PreviewViewer();
    m_scene = new PreviewScene(m_viewer);
    m_viewer->setScene(m_scene);

    // log edit widget
    m_textEditLog = new QPlainTextEdit;

    // progress widget
    m_progressBar = new QProgressBar;
    m_progressBar->setMinimum(0);
    m_progressBar->setMaximum(100);
    statusBar->addWidget(m_progressBar);

    // initialize Dock Manager
    CDockManager::setConfigFlag(CDockManager::OpaqueSplitterResize, true);
    CDockManager::setConfigFlag(CDockManager::XmlCompressionEnabled, false);
    CDockManager::setConfigFlag(CDockManager::FocusHighlighting, true);
    m_dockManager = new CDockManager(this);

    CDockWidget* centralDockWidget = new CDockWidget(tr("Canvas"));
    centralDockWidget->setWidget(m_viewer);
    CDockAreaWidget* centralDockArea = m_dockManager->setCentralWidget(centralDockWidget);
    centralDockArea->setAllowedAreas(DockWidgetArea::OuterDockAreas);

    // create log dock panel
    m_logDockWidget = new CDockWidget(tr("Log"));
    m_logDockWidget->setWidget(m_textEditLog);
    CDockAreaWidget* bottomDockArea = m_dockManager->addDockWidget(BottomDockWidgetArea, m_logDockWidget);

    m_actionZoomIn->connect(m_actionZoomIn, &QAction::triggered, this, [=] {
        m_viewer->zoomIn();
    });
    m_actionZoomOut->connect(m_actionZoomOut, &QAction::triggered, this, [=] {
        m_viewer->zoomOut();
    });
    connect(this, &PreviewWindow::addPathSignal, this, &PreviewWindow::onAddPath);
    connect(this, &PreviewWindow::addLineSignal, this, &PreviewWindow::onAddLine);
    connect(this, &PreviewWindow::setTitleSignal, this, &PreviewWindow::onSetTitle);
    connect(this, &PreviewWindow::addProgressSignal, this, &PreviewWindow::onAddProgress);
    connect(this, QOverload<qreal>::of(&PreviewWindow::setProgressSignal), 
        this, QOverload<qreal>::of(&PreviewWindow::onSetProgress));
    connect(this, QOverload<quint32, qreal>::of(&PreviewWindow::setProgressSignal), 
        this, QOverload<quint32, qreal>::of(&PreviewWindow::onSetProgress));
    connect(this, &PreviewWindow::addMessageSignal, this, &PreviewWindow::onAddMessage);
    connect(this, &PreviewWindow::addPointsSignal, this, &PreviewWindow::onAddPoints);
    connect(this, &PreviewWindow::addPointSignal, this, &PreviewWindow::onAddPoint);

    resize(QDesktopWidget().availableGeometry(this).size() * 0.7);

    updatePreviewArea();
}

PreviewWindow::~PreviewWindow()
{
}

qreal PreviewWindow::progress() const
{
    return m_progress;
}

void PreviewWindow::setTitle(const QString& msg)
{
    emit setTitleSignal(msg);
}

void PreviewWindow::addPath(const QPainterPath& path, QPen pen, const QString& label)
{
    emit addPathSignal(path, pen, label);
}

void PreviewWindow::addLine(const QLineF& line, QPen pen, const QString& label)
{
    emit addLineSignal(line, pen, label);
}

void PreviewWindow::addPoints(const QList<QPointF>& points, QPen pen, int style)
{
    emit addPointsSignal(points, pen, style);
}

void PreviewWindow::addPoint(const QPointF& point, QPen pen, int style)
{
    emit addPointSignal(point, pen, style);
}

void PreviewWindow::updatePreviewArea()
{
    QSizeF viewSize = m_viewer->viewport()->size();
    QSizeF sceneSize = m_viewer->scene()->sceneRect().size();
    qreal scaleX = viewSize.width() / sceneSize.width();
    qreal scaleY = viewSize.height() / sceneSize.height();
    qreal scale = qMax(scaleX, scaleY);
    m_viewer->setTransform(QTransform::fromScale(scale, scale));
    
}

void PreviewWindow::reset()
{
    m_textEditLog->clear();
    m_progressBar->setValue(0);
    m_viewer->scene()->clear();
    m_viewer->scene()->addRect(LaserApplication::device->boundRectMachining());
    
    resetProgress();
    updatePreviewArea();
    m_viewer->reset();
}

void PreviewWindow::resetProgress()
{
    m_progress = 0.0;
    m_progressQuotas.clear();
    m_sumQuotas = 0;
}

void PreviewWindow::registerProgressCode(quint32 code, qreal quota)
{
    m_progressQuotas[code] = quota;
    m_sumQuotas = 0;
    for (QMap<quint32, qreal>::ConstIterator i = m_progressQuotas.constBegin(); i != m_progressQuotas.constEnd(); i++)
    {
        m_sumQuotas += i.value();
    }
}

void PreviewWindow::registerProgressCode(void* ptr, qreal quota)
{
    registerProgressCode((quint32)ptr, quota);
}

void PreviewWindow::addProgress(quint32 code, qreal deltaProgress, const QString& msg, bool ignoreQuota)
{
    emit addProgressSignal(code, deltaProgress, msg, ignoreQuota);
}

void PreviewWindow::addProgress(void* code, qreal deltaProgress, const QString& msg, bool ignoreQuota)
{
    addProgress((quint32)code, deltaProgress, msg, ignoreQuota);
}

void PreviewWindow::setProgress(qreal progress)
{
    emit setProgressSignal(progress);
}

void PreviewWindow::setProgress(quint32 code, qreal progress)
{
    emit setProgressSignal(code, progress);
}

void PreviewWindow::setProgress(void* ptr, qreal progress)
{
    setProgress((quint32)ptr, progress);
}

void PreviewWindow::addMessage(const QString& message)
{
    emit addMessageSignal(message);
}

void PreviewWindow::onAddPath(const QPainterPath& path, QPen pen, const QString& label)
{
    m_scene->addPath(path, pen, label);
    updatePreviewArea();
}

void PreviewWindow::onAddLine(const QLineF& line, QPen pen, const QString& label)
{
    m_scene->addLine(line, pen, label);
    updatePreviewArea();
}

void PreviewWindow::onAddPoints(const QList<QPointF>& points, QPen pen, int style)
{
    m_scene->addPoints(points, pen, style);
    updatePreviewArea();
}

void PreviewWindow::onAddPoint(const QPointF& point, QPen pen, int style)
{
    m_scene->addPoint(point, pen, style);
    updatePreviewArea();
}

void PreviewWindow::onSetTitle(const QString& msg)
{
    m_logDockWidget->setWindowTitle(msg);
    m_textEditLog->appendPlainText(msg);
}

void PreviewWindow::onAddProgress(quint32 code, qreal deltaProgress, const QString& msg, bool ignoreQuota)
{
    if (!m_progressQuotas.contains(code))
        return;

    if (!ignoreQuota)
    {
        qreal quota = m_progressQuotas[code];
        deltaProgress = deltaProgress * quota;
    }
    onSetProgress(qMin(m_progress + deltaProgress, 1.0));

    if (!msg.isEmpty())
    {
        onAddMessage(msg);
    }
}

void PreviewWindow::onSetProgress(qreal progress)
{
    m_progress = progress;
    m_progressBar->setValue(qRound(progress * 100));
    emit progressUpdated(m_progress);
}

void PreviewWindow::onSetProgress(quint32 code, qreal progress)
{
    if (!m_progressQuotas.contains(code))
        return;

    qreal quota = m_progressQuotas[code];
    onSetProgress(qMin(progress * quota, 1.0));
}

void PreviewWindow::onAddMessage(const QString& message)
{
    qLogD << message;
    m_textEditLog->appendPlainText(message);
}
