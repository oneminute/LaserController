#include "PreviewWindow.h"

#include "common/common.h"
#include "common/Config.h"
#include "scene/LaserPrimitive.h"
#include <LaserApplication.h>
#include <laser/LaserDevice.h>
#include <widget/PreviewScene.h>
#include <QDesktopWidget>
#include <QEventLoop>
#include <QMenuBar>
#include <QPen>
#include <QPointer>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QToolBar>
#include <QTreeView>
#include <QtMath>
#include <QStatusBar>
#include <DockAreaTabBar.h>
#include <DockAreaTitleBar.h>
#include <DockAreaWidget.h>
#include <DockComponentsFactory.h>
#include <DockContainerWidget.h>
#include <DockSplitter.h>
#include <DockWidgetTab.h>
#include "widget/PreviewViewer.h"
#include "widget/ProgressBarDelegate.h"
#include "task/ProgressModel.h"

using namespace ads;

PreviewWindow::PreviewWindow(QWidget* parent)
    : QMainWindow(parent)
{
    this->setWindowModality(Qt::WindowModality::WindowModal);
    //this->setWindowFlags(Qt::WindowStaysOnTopHint);

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

    // progress tree widget
    m_treeViewProgress = new QTreeView;
    m_treeViewProgress->setModel(LaserApplication::progressModel);
    ProgressBarDelegate* progressBarDelegate = new ProgressBarDelegate(m_treeViewProgress);
    m_treeViewProgress->setItemDelegateForColumn(1, progressBarDelegate);

    // initialize Dock Manager
    CDockManager::setConfigFlag(CDockManager::OpaqueSplitterResize, true);
    CDockManager::setConfigFlag(CDockManager::XmlCompressionEnabled, false);
    CDockManager::setConfigFlag(CDockManager::FocusHighlighting, true);
    m_dockManager = new CDockManager(this);

    m_canvasDockWidget = new CDockWidget(tr("Canvas"));
    m_canvasDockWidget->setWidget(m_viewer);
    //CDockAreaWidget* centralDockArea = m_dockManager->setCentralWidget(centralDockWidget);
    CDockAreaWidget* canvasDockArea = m_dockManager->addDockWidget(RightDockWidgetArea, m_canvasDockWidget);
    //centralDockArea->setAllowedAreas(DockWidgetArea::OuterDockAreas);

    // create progress tree view dock panel
    m_progressDockWidget = new CDockWidget(tr("Progress"));
    m_progressDockWidget->setWidget(m_treeViewProgress);
    CDockAreaWidget* progressDockArea = m_dockManager->addDockWidget(LeftDockWidgetArea, m_progressDockWidget);

    // create log dock panel
    m_logDockWidget = new CDockWidget(tr("Log"));
    m_logDockWidget->setWidget(m_textEditLog);
    CDockAreaWidget* logDockArea = m_dockManager->addDockWidget(BottomDockWidgetArea, m_logDockWidget, progressDockArea);

    //canvasDockArea->resize(QSize(600, 600));
    //progressDockArea->resize(QSize(600, 300));
    //logDockArea->resize(QSize(600, 300));
    m_canvasDockWidget->resize(QSize(600, 600));
    m_progressDockWidget->resize(QSize(600, 300));
    m_logDockWidget->resize(QSize(600, 300));
    m_progressDockWidget->setMinimumWidth(500);

    internal::findParent<QSplitter*>(canvasDockArea)->setHandleWidth(Config::Ui::splitterHandleWidth());
    internal::findParent<QSplitter*>(progressDockArea)->setHandleWidth(Config::Ui::splitterHandleWidth());
    internal::findParent<QSplitter*>(logDockArea)->setHandleWidth(Config::Ui::splitterHandleWidth());

    m_actionZoomIn->connect(m_actionZoomIn, &QAction::triggered, this, [=] {
        m_viewer->zoomIn();
    });
    m_actionZoomOut->connect(m_actionZoomOut, &QAction::triggered, this, [=] {
        m_viewer->zoomOut();
    });
    connect(this, &PreviewWindow::addPathSignal, this, &PreviewWindow::onAddPath);
    connect(this, &PreviewWindow::addLineSignal, this, &PreviewWindow::onAddLine);
    connect(this, &PreviewWindow::setTitleSignal, this, &PreviewWindow::onSetTitle);
    connect(this, &PreviewWindow::addPointsSignal, this, &PreviewWindow::onAddPoints);
    connect(this, &PreviewWindow::addPointSignal, this, &PreviewWindow::onAddPoint);
    connect(this, &PreviewWindow::addMessageSignal, this, &PreviewWindow::onAddMessage);
    connect(&m_updateTimer, &QTimer::timeout, this, &PreviewWindow::updatePreviewArea);

    resize(QDesktopWidget().availableGeometry(this).size() * 0.7);

    intendUpdate();
}

PreviewWindow::~PreviewWindow()
{
}

void PreviewWindow::intendUpdate()
{
    if (!m_updateTimer.isActive())
    {
        m_updateTimer.start(1500);
    }
}

void PreviewWindow::setTitle(const QString& msg)
{
    emit setTitleSignal(msg);
}

void PreviewWindow::addMessage(const QString& message)
{
    emit addMessageSignal(message);
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
    m_updateTimer.stop();
    qLogD << "update preview area";
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
    
    intendUpdate();
    m_viewer->reset();
}

void PreviewWindow::onAddPath(const QPainterPath& path, QPen pen, const QString& label)
{
    m_scene->addPath(path, pen, label);
    intendUpdate();
}

void PreviewWindow::onAddLine(const QLineF& line, QPen pen, const QString& label)
{
    m_scene->addLine(line, pen, label);
    intendUpdate();
}

void PreviewWindow::onAddPoints(const QList<QPointF>& points, QPen pen, int style)
{
    m_scene->addPoints(points, pen, style);
    intendUpdate();
}

void PreviewWindow::onAddPoint(const QPointF& point, QPen pen, int style)
{
    m_scene->addPoint(point, pen, style);
    intendUpdate();
}

void PreviewWindow::onSetTitle(const QString& msg)
{
    m_logDockWidget->setWindowTitle(msg);
    m_textEditLog->appendPlainText(msg);
}

void PreviewWindow::onAddMessage(const QString& message)
{
    //qLogD << message;
    m_textEditLog->appendPlainText(message);
}

