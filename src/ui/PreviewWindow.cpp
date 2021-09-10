#include "PreviewWindow.h"

#include "common/common.h"
#include "scene/LaserPrimitive.h"
#include <LaserApplication.h>
#include <laser/LaserDevice.h>
#include <QDesktopWidget>
#include <QEventLoop>
#include <QMenuBar>
#include <QPen>
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

    m_viewer->scene()->addRect(LaserApplication::device->boundRectMachining());
    updatePreviewArea();
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
    qLogD << "progress: " << progress;
    m_progressBar->setValue(progress);
}

void PreviewWindow::finished()
{
}

void PreviewWindow::addPrimitive(LaserPrimitive* primitive)
{
    m_viewer->scene()->addItem(primitive);
    updatePreviewArea();
}

void PreviewWindow::addPath(const QPainterPath& path, QPen pen, const QString& label)
{
    QGraphicsPathItem* item = m_viewer->scene()->addPath(path);
    pen.setCosmetic(true);
    item->setPen(pen);
    
    if (!label.isEmpty())
    {
        QPointF pos = path.pointAtPercent(0);
        QGraphicsTextItem* textItem = m_viewer->scene()->addText(label);
        textItem->setDefaultTextColor(pen.color());
        textItem->setPos(pos);
    }
    updatePreviewArea();
}

void PreviewWindow::addLine(const QLineF& line, QPen pen, const QString& label)
{
    QGraphicsLineItem* item = m_viewer->scene()->addLine(line);
    pen.setCosmetic(true);
    item->setPen(pen);

    QGraphicsEllipseItem* circleItem = m_viewer->scene()->addEllipse(
        line.p1().x() - 50, line.p1().y() - 50, 100, 100, pen);
    circleItem->setParentItem(item);

    QVector2D dir(line.p1() - line.p2());
    dir.normalize();
    QTransform t1, t2;
    t1.rotate(15);
    t2.rotate(-15);
    QLineF arrowLine(QPointF(0, 0), (dir * 150).toPointF());
    QLineF arrowLine1 = t1.map(arrowLine);
    QLineF arrowLine2 = t2.map(arrowLine);
    arrowLine1.translate(line.p2());
    arrowLine2.translate(line.p2());
    QLineF arrowLine3(arrowLine1.p2(), arrowLine2.p2());
    //QPointF arrowPt1 = line.p2() + (dir * 150).toPointF() + (vDir * 50).toPointF();
    //QPointF arrowPt2 = line.p2() + (dir * 150).toPointF() + (-vDir * 50).toPointF();
    QGraphicsLineItem* arrow1 = m_viewer->scene()->addLine(arrowLine1, pen);
    QGraphicsLineItem* arrow2 = m_viewer->scene()->addLine(arrowLine2, pen);
    QGraphicsLineItem* arrow3 = m_viewer->scene()->addLine(arrowLine3, pen);
    arrow1->setParentItem(item);
    arrow2->setParentItem(item);
    arrow3->setParentItem(item);
    
    if (!label.isEmpty())
    {
        QPointF pos = (line.p1() + line.p2()) / 2;
        QGraphicsTextItem* textItem = m_viewer->scene()->addText(label);
        textItem->setParentItem(item);
        //textItem->setRotation(360 - line.angle());
        //QRectF bounding = textItem->sceneBoundingRect();
        //pos -= QPointF(bounding.width() / 2, bounding.height() / 2);
        textItem->setDefaultTextColor(pen.color());
        textItem->setPos(pos);
        QFont font = textItem->font();
        font.setPointSizeF(font.pointSizeF() * 1.5);
        textItem->setFont(font);
    }
    updatePreviewArea();
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
