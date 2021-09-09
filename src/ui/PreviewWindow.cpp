#include "PreviewWindow.h"

#include "common/common.h"
#include <QDesktopWidget>
#include <QEventLoop>
#include <QMenuBar>
#include <QPointer>
#include <QToolBar>
#include <QStatusBar>
#include <DockAreaTabBar.h>
#include <DockAreaTitleBar.h>
#include <DockAreaWidget.h>
#include <DockComponentsFactory.h>
#include "widget/PreviewViewer.h"

using namespace ads;

PreviewWindow::PreviewWindow(QWidget* parent)
    : QMainWindow(parent)
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
    m_viewer = new PreviewViewer;

    // log edit widget
    m_textEditLog = new QPlainTextEdit;

    // progress widget
    m_progressBar = new QProgressBar;
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

    resize(QDesktopWidget().availableGeometry(this).size() * 0.7);
}

PreviewWindow::~PreviewWindow()
{
}

int PreviewWindow::exec()
{
    bool deleteOnClose = testAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_DeleteOnClose, false);

    bool wasShowModal = testAttribute(Qt::WA_ShowModal);
    setAttribute(Qt::WA_ShowModal, true);

    show();

    QEventLoop eventLoop;
    (void) eventLoop.exec(QEventLoop::DialogExec);

    setAttribute(Qt::WA_ShowModal, wasShowModal);

    int res = result();
    if (deleteOnClose)
        delete this;
    return res;
}

int PreviewWindow::result() const
{
    return 0;
}

void PreviewWindow::addMessage(const QString& msg)
{
    qLogD << msg;
    m_textEditLog->appendPlainText(msg);
}

void PreviewWindow::setTitle(const QString& msg)
{
    m_logDockWidget->setWindowTitle(msg);
    m_textEditLog->appendPlainText(msg);
}

void PreviewWindow::setProgress(float progress)
{
    m_progressBar->setValue(progress);
}

void PreviewWindow::finished()
{
}
