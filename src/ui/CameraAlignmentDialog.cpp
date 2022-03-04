#include "CameraAlignmentDialog.h"

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QPen>
#include <QPushButton>
#include <QSpinBox>
#include <QRadioButton>
#include <QTableWidget>

#include "common/Config.h"
#include "camera/CameraController.h"
#include "camera/DistortionCalibrator.h"
#include "LaserApplication.h"
#include "laser/LaserDevice.h"
#include "widget/ImageViewer.h"
#include "scene/LaserDocument.h"
#include "scene/LaserScene.h"
#include "widget/GraphicsViewEx.h"
#include "ui/LaserControllerWindow.h"

CameraAlignmentDialog::CameraAlignmentDialog(CameraController* cameraController, DistortionCalibrator* calibrator, QWidget* parent)
    : WizardDialog(parent)
    , m_doc(nullptr)
    , m_cameraController(cameraController)
    , m_calibrator(calibrator)
    , m_waitingForImage(false)
{
    installEventFilter(this);
    Config::Device::startFromItem()->push();
    Config::Export::imageQualityItem()->push();

    Config::Export::imageQualityItem()->setValue(IQ_Normal, SS_DIRECTLY, this);

    m_labelStatus = new QLabel;
    m_labelStatus->setText(tr("Disconnected"));
    m_frameStatus = new QLabel;
    QHBoxLayout* layout = buttonsLayout();
    layout->insertStretch(0);
    layout->insertWidget(0, m_labelStatus);
    layout->insertWidget(1, m_frameStatus);

    m_page1 = new WizardDialogPage(tr("Print Board"));
    m_page1Introduction = new QLabel;
    m_page1Introduction->setText(tr(
        "<h1>Introduction</h1>\n"
        "<ol>"
        "<li>Modify the parameters bellow to generate a alignment document;</li>"
        "<li>Then click \"Generate\" button to preview the document;</li>"
        "<li>When you have done the ajustment, click \"Start\" button to print the document;</li>"
        "</ol>"
    ));

    QFormLayout* page1ParamsLayout = new QFormLayout;
    m_page1SpinBoxHMargin = new QSpinBox;
    m_page1SpinBoxVMargin = new QSpinBox;
    m_page1SpinBoxMarkSize = new QSpinBox;
    m_page1DoubleSpinBoxCuttingSpeed = new QDoubleSpinBox;
    m_page1DoubleSpinBoxCuttingMinPower = new QDoubleSpinBox;
    m_page1DoubleSpinBoxCuttingRunPower = new QDoubleSpinBox;
    m_page1DoubleSpinBoxFillingSpeed = new QDoubleSpinBox;
    m_page1DoubleSpinBoxFillingPower = new QDoubleSpinBox;
    m_page1DoubleSpinBoxFillingInterval = new QDoubleSpinBox;

    m_page1SpinBoxHMargin->setValue(10);
    m_page1SpinBoxHMargin->setMinimum(40);
    m_page1SpinBoxHMargin->setMaximum(100);
    m_page1SpinBoxVMargin->setValue(10);
    m_page1SpinBoxVMargin->setMinimum(10);
    m_page1SpinBoxVMargin->setMaximum(100);
    m_page1SpinBoxMarkSize->setValue(30);
    m_page1SpinBoxMarkSize->setMinimum(20);
    m_page1SpinBoxMarkSize->setMaximum(80);

    m_page1DoubleSpinBoxCuttingSpeed->setMinimum(1);
    m_page1DoubleSpinBoxCuttingSpeed->setMaximum(2000);
    m_page1DoubleSpinBoxCuttingSpeed->setValue(100);

    m_page1DoubleSpinBoxCuttingMinPower->setMinimum(0);
    m_page1DoubleSpinBoxCuttingMinPower->setMaximum(100);
    m_page1DoubleSpinBoxCuttingMinPower->setValue(5);

    m_page1DoubleSpinBoxCuttingRunPower->setMinimum(0);
    m_page1DoubleSpinBoxCuttingRunPower->setMaximum(100);
    m_page1DoubleSpinBoxCuttingRunPower->setValue(12);

    m_page1DoubleSpinBoxFillingSpeed->setMinimum(1);
    m_page1DoubleSpinBoxFillingSpeed->setMaximum(2000);
    m_page1DoubleSpinBoxFillingSpeed->setValue(200);

    m_page1DoubleSpinBoxFillingPower->setMinimum(0);
    m_page1DoubleSpinBoxFillingPower->setMaximum(100);
    m_page1DoubleSpinBoxFillingPower->setValue(12);

    m_page1DoubleSpinBoxFillingInterval->setMinimum(0);
    m_page1DoubleSpinBoxFillingInterval->setMaximum(2000);
    m_page1DoubleSpinBoxFillingInterval->setValue(70);

    page1ParamsLayout->setLabelAlignment(Qt::AlignRight);
    page1ParamsLayout->addRow(tr("Horizontal Margin"), m_page1SpinBoxHMargin);
    page1ParamsLayout->addRow(tr("Vertical Margin"), m_page1SpinBoxVMargin);
    page1ParamsLayout->addRow(tr("Mark Size"), m_page1SpinBoxMarkSize);
    page1ParamsLayout->addRow(tr("Cutting Speed(mm/s)"), m_page1DoubleSpinBoxCuttingSpeed);
    page1ParamsLayout->addRow(tr("Cutting Min Power(%)"), m_page1DoubleSpinBoxCuttingMinPower);
    page1ParamsLayout->addRow(tr("Cutting Run Power(%)"), m_page1DoubleSpinBoxCuttingRunPower);
    page1ParamsLayout->addRow(tr("Filling Speed(mm/s)"), m_page1DoubleSpinBoxFillingSpeed);
    page1ParamsLayout->addRow(tr("Filling Run Power(%)"), m_page1DoubleSpinBoxFillingPower);
    page1ParamsLayout->addRow(tr("Filling Interval(um)"), m_page1DoubleSpinBoxFillingInterval);

    m_page1ButtonGenerate = new QPushButton(tr("Generate"));
    connect(m_page1ButtonGenerate, &QPushButton::clicked, this, &CameraAlignmentDialog::generate);
    m_page1ButtonStart = new QPushButton(tr("Start"));
    connect(m_page1ButtonStart, &QPushButton::clicked, this, &CameraAlignmentDialog::start);
    m_page1ButtonStop = new QPushButton(tr("Stop"));
    connect(m_page1ButtonStop, &QPushButton::clicked, this, &CameraAlignmentDialog::stop);

    QHBoxLayout* page1ButtonsLayout = new QHBoxLayout;
    page1ButtonsLayout->addWidget(m_page1ButtonGenerate);
    page1ButtonsLayout->addWidget(m_page1ButtonStart);
    page1ButtonsLayout->addWidget(m_page1ButtonStop);

    QVBoxLayout* page1LeftLayout = new QVBoxLayout;
    page1LeftLayout->addLayout(page1ParamsLayout);
    page1LeftLayout->addLayout(page1ButtonsLayout);

    m_page1Viewer = new QGraphicsView;
    m_page1Scene = new QGraphicsScene;
    m_page1Scene->setSceneRect(LaserApplication::device->layoutRect());
    m_page1Viewer->setScene(m_page1Scene);
    m_page1Viewer->setResizeAnchor(QGraphicsView::ViewportAnchor::AnchorViewCenter);
    m_page1Viewer->setAlignment(Qt::AlignCenter);
    m_page1Viewer->fitInView(m_page1Scene->sceneRect(), Qt::KeepAspectRatio);

    QVBoxLayout* page1RightLayout = new QVBoxLayout;
    page1RightLayout->addWidget(m_page1Viewer);

    QHBoxLayout* page1BottomLayout = new QHBoxLayout;
    page1BottomLayout->addLayout(page1LeftLayout);
    page1BottomLayout->addLayout(page1RightLayout);
    page1BottomLayout->setStretch(0, 0);
    page1BottomLayout->setStretch(1, 1);
    QVBoxLayout* page1Layout = new QVBoxLayout;
    page1Layout->addWidget(m_page1Introduction);
    page1Layout->addLayout(page1BottomLayout);
    page1Layout->setStretch(0, 0);
    page1Layout->setStretch(1, 1);
    m_page1->setLayout(page1Layout);

    m_page2 = new WizardDialogPage(tr("Select Points"));
    m_page2Introduction = new QLabel;
    m_page2Introduction->setText(tr(
        "<h1>Select points</h1>\n"
        "<ol>"
        "</ol>"
    ));
    m_page2ButtonCapture = new QPushButton(tr("Capture"));
    connect(m_page2ButtonCapture, &QPushButton::clicked, this, &CameraAlignmentDialog::capture);
    m_page2ButtonCalculate = new QPushButton(tr("Calculate"));
    connect(m_page2ButtonCalculate, &QPushButton::clicked, this, &CameraAlignmentDialog::calculate);
    QGridLayout* page2RadioButtonsLayout = new QGridLayout;
    QRadioButton* page2RadioButton1 = new QRadioButton;
    page2RadioButton1->setProperty("markIndex", 0);
    page2RadioButtonsLayout->addWidget(page2RadioButton1, 0, 0);
    QRadioButton* page2RadioButton2 = new QRadioButton;
    page2RadioButton2->setProperty("markIndex", 1);
    page2RadioButtonsLayout->addWidget(page2RadioButton2, 0, 2);
    QRadioButton* page2RadioButton3 = new QRadioButton;
    page2RadioButton3->setProperty("markIndex", 4);
    page2RadioButtonsLayout->addWidget(page2RadioButton3, 1, 1);
    QRadioButton* page2RadioButton4 = new QRadioButton;
    page2RadioButton4->setProperty("markIndex", 2);
    page2RadioButtonsLayout->addWidget(page2RadioButton4, 2, 0);
    QRadioButton* page2RadioButton5 = new QRadioButton;
    page2RadioButton5->setProperty("markIndex", 3);
    page2RadioButtonsLayout->addWidget(page2RadioButton5, 2, 2);
    connect(page2RadioButton1, &QRadioButton::toggled, this, &CameraAlignmentDialog::onMarkIndexChanged);
    connect(page2RadioButton2, &QRadioButton::toggled, this, &CameraAlignmentDialog::onMarkIndexChanged);
    connect(page2RadioButton3, &QRadioButton::toggled, this, &CameraAlignmentDialog::onMarkIndexChanged);
    connect(page2RadioButton4, &QRadioButton::toggled, this, &CameraAlignmentDialog::onMarkIndexChanged);
    connect(page2RadioButton5, &QRadioButton::toggled, this, &CameraAlignmentDialog::onMarkIndexChanged);
    QVBoxLayout* page2ButtonsLayout = new QVBoxLayout;
    page2ButtonsLayout->addWidget(m_page2ButtonCapture);
    page2ButtonsLayout->addWidget(m_page2ButtonCalculate);
    page2ButtonsLayout->addLayout(page2RadioButtonsLayout);
    QHBoxLayout* page2TitleLayout = new QHBoxLayout;
    page2TitleLayout->addWidget(m_page2Introduction);
    page2TitleLayout->addLayout(page2ButtonsLayout);
    page2TitleLayout->setStretch(0, 1);
    page2TitleLayout->setStretch(1, 0);
    m_page2Viewer = new GraphicsViewEx;
    m_page2Scene = new QGraphicsScene;
    m_page2Scene->setSceneRect(QRect(QPoint(0, 0), Config::Camera::resolution()));
    connect(m_page2Viewer, &GraphicsViewEx::clicked, this, &CameraAlignmentDialog::setMarkPos);
    m_page2Viewer->setScene(m_page2Scene);
    m_page2Viewer->fitInView(m_page2Scene->sceneRect(), Qt::KeepAspectRatio);
    QVBoxLayout* page2Layout = new QVBoxLayout;
    page2Layout->addLayout(page2TitleLayout);
    page2Layout->addWidget(m_page2Viewer);
    page2Layout->setStretch(0, 0);
    page2Layout->setStretch(1, 1);
    m_page2->setLayout(page2Layout);

    m_page3 = new WizardDialogPage(tr("Verify"));
    m_page3Introduction = new QLabel;
    m_page3Introduction->setText(tr(
        "<h1>Verify</h1>\n"
        "<ol>"
        "</ol>"
    ));
    m_page3ButtonVerify = new QPushButton(tr("Verify"));
    connect(m_page3ButtonVerify, &QPushButton::clicked, this, &CameraAlignmentDialog::verify);
    m_page3ButtonSave = new QPushButton(tr("Save"));
    connect(m_page3ButtonSave, &QPushButton::clicked, this, &CameraAlignmentDialog::save);
    QVBoxLayout* page3ButtonsLayout = new QVBoxLayout;
    page3ButtonsLayout->addWidget(m_page3ButtonVerify);
    page3ButtonsLayout->addWidget(m_page3ButtonSave);
    QHBoxLayout* page3TitleLayout = new QHBoxLayout;
    page3TitleLayout->addWidget(m_page3Introduction);
    page3TitleLayout->addLayout(page3ButtonsLayout);
    page3TitleLayout->setStretch(0, 1);
    page3TitleLayout->setStretch(1, 0);
    m_page3Viewer = new GraphicsViewEx;
    m_page3Scene = new QGraphicsScene;
    m_page3Scene->setSceneRect(LaserApplication::device->layoutRect());
    m_page3Viewer->setScene(m_page3Scene);
    m_page3Viewer->setResizeAnchor(QGraphicsView::ViewportAnchor::AnchorViewCenter);
    m_page3Viewer->setAlignment(Qt::AlignCenter);
    QVBoxLayout* page3Layout = new QVBoxLayout;
    page3Layout->addLayout(page3TitleLayout);
    page3Layout->addWidget(m_page3Viewer);
    page3Layout->setStretch(0, 0);
    page3Layout->setStretch(1, 1);
    m_page3->setLayout(page3Layout);

    connect(m_cameraController, &CameraController::connected, this, &CameraAlignmentDialog::onCameraConnected);
    connect(m_cameraController, &CameraController::disconnected, this, &CameraAlignmentDialog::onCameraDisconnected);
    m_cameraController->start();
    m_cameraController->registerSubscriber(this);

    setLeftLayoutWidth(120);
    appendPage(m_page1);
    appendPage(m_page2);
    appendPage(m_page3);
    updatePage();
    resize(1280, 800);

    page2RadioButton1->setChecked(true);
    connect(LaserApplication::device, &LaserDevice::workStateUpdated, this, &CameraAlignmentDialog::onDeviceStateChanged);
    onCameraConnected();
}

CameraAlignmentDialog::~CameraAlignmentDialog()
{
    m_cameraController->unregisterSubscriber(this);
}

bool CameraAlignmentDialog::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == Event_CameraFrame) {
        CameraFrameEvent* frameEvent = static_cast<CameraFrameEvent*>(event);
        m_frameStatus->setText(tr("fps: %1, duration: %2").arg(1000.0 / frameEvent->duration(), 0, 'f', 3).arg(frameEvent->duration()));

        if (!m_waitingForImage)
            return true;

        m_waitingForImage = false;
        m_calibrator->undistortImage(frameEvent->processedImage());

        if (currentIndex() == 1)
        {
            QImage image(frameEvent->processedImage().data, frameEvent->processedImage().cols, frameEvent->processedImage().rows, frameEvent->processedImage().step, QImage::Format_RGB888);
            QGraphicsPixmapItem* pixmapItem = m_page2Scene->addPixmap(QPixmap::fromImage(image));
            m_page2Viewer->fitInView(pixmapItem, Qt::KeepAspectRatio);
        }
        else if (currentIndex() == 2)
        {
            m_page3Scene->clear();

            QPen pen(Qt::gray, 2);
            pen.setCosmetic(true);
            m_page3Scene->clear();
            m_page3Scene->addRect(LaserApplication::device->layoutRect())->setPen(pen);

            cv::Mat perspected;
            m_calibrator->perspective(frameEvent->processedImage(), perspected);
            m_calibrator->alignToCanvas(perspected, m_page3Scene);

            int markSize = m_page1SpinBoxMarkSize->value() * 1000;
            int halfMarkSize = markSize / 2;

            pen.setColor(Qt::blue);
            pen.setWidth(1);
            QRect rect;
            QPainterPath painterPath;

            // draw top left
            addMarkToScene(halfMarkSize, m_mark0, pen, m_page3Scene, rect, painterPath);

            // draw top right
            addMarkToScene(halfMarkSize, m_mark1, pen, m_page3Scene, rect, painterPath);

            // draw bottom left
            addMarkToScene(halfMarkSize, m_mark2, pen, m_page3Scene, rect, painterPath);

            // draw bottom right
            addMarkToScene(halfMarkSize, m_mark3, pen, m_page3Scene, rect, painterPath);

            // draw center
            addMarkToScene(halfMarkSize, m_mark4, pen, m_page3Scene, rect, painterPath);
            m_page3Viewer->fitInView(m_page3Scene->sceneRect(), Qt::KeepAspectRatio);
        }        
        return true;
    }
    else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}
void CameraAlignmentDialog::closeEvent(QCloseEvent* e)
{
    if (m_doc)
        LaserApplication::mainWindow->closeDocument();
    Config::Device::startFromItem()->pop();
    Config::Export::imageQualityItem()->pop();
}

QPainterPath CameraAlignmentDialog::quarterCircle(const QPoint& center, int radius)
{
    QPainterPath path;
    path.moveTo(center);
    path.lineTo(center + QPoint(0, -radius));
    path.lineTo(center + QPoint(radius, 0));
    path.lineTo(center + QPoint(-radius, 0));
    path.lineTo(center + QPoint(0, radius));
    path.lineTo(center);
    return path;
}

void CameraAlignmentDialog::setupLayers(LaserLayer* layer0, LaserLayer* layer1)
{
    layer0->setType(LLT_CUTTING);
    layer0->setCuttingRunSpeed(m_page1DoubleSpinBoxCuttingSpeed->value() * 1000);
    layer0->setCuttingMinSpeedPower(m_page1DoubleSpinBoxCuttingMinPower->value() * 10);
    layer0->setCuttingRunSpeedPower(m_page1DoubleSpinBoxCuttingRunPower->value() * 10);

    layer1->setType(LLT_FILLING);
    layer1->setEngravingEnableCutting(false);
    layer1->setFillingEnableCutting(false);
    layer1->setEngravingRunSpeed(m_page1DoubleSpinBoxFillingSpeed->value() * 1000);
    layer1->setEngravingLaserPower(m_page1DoubleSpinBoxFillingPower->value() * 10);
    layer1->setEngravingMinSpeedPower(0);
    layer1->setEngravingRunSpeedPower(980);
    layer1->setEngravingRowInterval(m_page1DoubleSpinBoxFillingInterval->value());
}

void CameraAlignmentDialog::addMarkToScene(int halfMarkSize, 
    const QPoint& center, const QPen& pen,
    QGraphicsScene* scene, QRect& rect, QPainterPath& path)
{
    int innerRadius = halfMarkSize * 3 / 4;
    rect = QRect(center + QPoint(-halfMarkSize, -halfMarkSize), center + QPoint(halfMarkSize, halfMarkSize));
    path = quarterCircle(center, innerRadius);
    scene->addEllipse(rect)->setPen(pen);
    scene->addPath(path)->setBrush(Qt::blue);
}

void CameraAlignmentDialog::addMarkToDocument(int index, const QRect& rect, const QPainterPath& painterPath)
{
    LaserScene* scene = LaserApplication::mainWindow->scene();
    LaserLayer* layer0 = m_doc->layers()[index * 2];
    LaserLayer* layer1 = m_doc->layers()[index * 2 + 1];
    LaserEllipse* circle = new LaserEllipse(rect, m_doc);
    LaserPath* path = new LaserPath(painterPath, m_doc);
    scene->addLaserPrimitive(circle, layer0, false);
    scene->addLaserPrimitive(path, layer1, false);
    setupLayers(layer0, layer1);
}

void CameraAlignmentDialog::generate()
{
    if (m_doc)
    {
        LaserApplication::mainWindow->closeDocument();
    }

    QPen pen(Qt::gray, 2);
    pen.setCosmetic(true);
    m_page1Scene->clear();
    m_page1Scene->addRect(LaserApplication::device->layoutRect())->setPen(pen);

    LaserApplication::mainWindow->newDocument();
    LaserScene* scene = LaserApplication::mainWindow->scene();
    m_doc = scene->document();
    //m_doc->setFinishRun(FT_BackToOrigin);
    m_doc->setFinishRun(FT_CurrentPos);
    
    int hMargin = m_page1SpinBoxHMargin->value() * 1000;
    int vMargin = m_page1SpinBoxVMargin->value() * 1000;
    int markSize = m_page1SpinBoxMarkSize->value() * 1000;
    int halfMarkSize = markSize / 2;

    pen.setColor(Qt::blue);
    pen.setWidth(1);
    QRect rect;
    QPainterPath painterPath;

    // draw top left
    m_mark0 = LaserApplication::device->mapFromQuadToCurrent(
        QPoint(hMargin + markSize / 2, vMargin + markSize / 2));
    addMarkToScene(halfMarkSize, m_mark0, pen, m_page1Scene, rect, painterPath);
    addMarkToDocument(0, rect, painterPath);

    // draw top right
    m_mark1 = LaserApplication::device->mapFromQuadToCurrent(
        QPoint(Config::SystemRegister::xMaxLength() - hMargin - markSize / 2, 
        vMargin + markSize / 2));
    addMarkToScene(halfMarkSize, m_mark1, pen, m_page1Scene, rect, painterPath);
    addMarkToDocument(1, rect, painterPath);

    // draw bottom left
    m_mark2 = LaserApplication::device->mapFromQuadToCurrent(
        QPoint(hMargin + markSize / 2, 
        Config::SystemRegister::yMaxLength() - vMargin - markSize / 2));
    addMarkToScene(halfMarkSize, m_mark2, pen, m_page1Scene, rect, painterPath);
    addMarkToDocument(2, rect, painterPath);

    // draw bottom right
    m_mark3 = LaserApplication::device->mapFromQuadToCurrent(
        QPoint(Config::SystemRegister::xMaxLength() - hMargin - markSize / 2, 
        Config::SystemRegister::yMaxLength() - vMargin - markSize / 2));
    addMarkToScene(halfMarkSize, m_mark3, pen, m_page1Scene, rect, painterPath);
    addMarkToDocument(3, rect, painterPath);

    // draw center
    m_mark4 = LaserApplication::device->mapFromQuadToCurrent(
        QPoint((Config::SystemRegister::xMaxLength() - 1) / 2, 
        (Config::SystemRegister::yMaxLength() - 1) / 2));
    addMarkToScene(halfMarkSize, m_mark4, pen, m_page1Scene, rect, painterPath);
    addMarkToDocument(4, rect, painterPath);

    LaserApplication::mainWindow->updateLayers();
}

void CameraAlignmentDialog::start()
{
    Config::Device::startFromItem()->setValue(SFT_AbsoluteCoords, SS_DIRECTLY, this);
    LaserApplication::mainWindow->startMachining();
}

void CameraAlignmentDialog::stop()
{
}

void CameraAlignmentDialog::capture()
{
    m_waitingForImage = true;
}

void CameraAlignmentDialog::calculate()
{
    if (m_items.count() != 5)
        return;

    std::vector<cv::Point2f> markPoints;
    std::vector<cv::Point2f> imagePoints;

    QPoint mark0 = LaserApplication::device->mapFromCurrentToQuad(m_mark0) / 1000;
    QPoint mark1 = LaserApplication::device->mapFromCurrentToQuad(m_mark1) / 1000;
    QPoint mark2 = LaserApplication::device->mapFromCurrentToQuad(m_mark2) / 1000;
    QPoint mark3 = LaserApplication::device->mapFromCurrentToQuad(m_mark3) / 1000;
    QPoint mark4 = LaserApplication::device->mapFromCurrentToQuad(m_mark4) / 1000;

    QRect layoutRect = LaserApplication::device->layoutRect();
    QSize resol = Config::Camera::resolution();
    qreal hFactor = resol.width() * 1000.0 / layoutRect.width();
    qreal vFactor = resol.height() * 1000.0 / layoutRect.height();

    markPoints.push_back(cv::Point2f(mark0.x() * hFactor, mark0.y() * vFactor));
    markPoints.push_back(cv::Point2f(mark1.x() * hFactor, mark1.y() * vFactor));
    markPoints.push_back(cv::Point2f(mark2.x() * hFactor, mark2.y() * vFactor));
    markPoints.push_back(cv::Point2f(mark3.x() * hFactor, mark3.y() * vFactor));
    markPoints.push_back(cv::Point2f(mark4.x() * hFactor, mark4.y() * vFactor));

    imagePoints.push_back(cv::Point2f(m_items[0]->pos().x(), m_items[0]->pos().y()));
    imagePoints.push_back(cv::Point2f(m_items[1]->pos().x(), m_items[1]->pos().y()));
    imagePoints.push_back(cv::Point2f(m_items[2]->pos().x(), m_items[2]->pos().y()));
    imagePoints.push_back(cv::Point2f(m_items[3]->pos().x(), m_items[3]->pos().y()));
    imagePoints.push_back(cv::Point2f(m_items[4]->pos().x(), m_items[4]->pos().y()));

    m_calibrator->calculateHomography(imagePoints, markPoints);
}

void CameraAlignmentDialog::verify()
{
    if (!m_calibrator->isHomographyValid())
        return;

    m_waitingForImage = true;
}

void CameraAlignmentDialog::save()
{
    m_calibrator->saveCoeffs();
}

void CameraAlignmentDialog::setMarkPos(const QPointF& pos)
{
    int halfRadius = 20;
    QGraphicsLineItem* currentMarkItem;
    if (m_items.contains(m_currentMarkIndex))
    {
        currentMarkItem = qgraphicsitem_cast<QGraphicsLineItem*>(m_items[m_currentMarkIndex]);
        //qLogD << currentMarkItem->pos();
        currentMarkItem->setPos(pos);
    }
    else
    {
        QPen pen(Qt::red, Qt::NoPen);
        pen.setCosmetic(true);
        QGraphicsLineItem* lineV = m_page2Scene->addLine(QLineF(QPointF(0, -halfRadius), QPointF(0, halfRadius)), pen);
        QGraphicsLineItem* lineH = m_page2Scene->addLine(QLineF(QPointF(-halfRadius, 0), QPointF(halfRadius, 0)), pen);
        QGraphicsEllipseItem* rect = m_page2Scene->addEllipse(QRect(QPoint(-halfRadius - 1, -halfRadius - 1), QPoint(halfRadius, halfRadius)), pen);
        lineH->setParentItem(lineV);
        rect->setParentItem(lineV);
        currentMarkItem = lineV;
        //qLogD << currentMarkItem->pos();
        currentMarkItem->setPos(pos);

        m_items.insert(m_currentMarkIndex, currentMarkItem);
    }
}

void CameraAlignmentDialog::onCameraConnected()
{
    m_labelStatus->setText(tr("Connected"));
    m_labelStatus->setStyleSheet("color: rgb(0, 255, 0)");
}

void CameraAlignmentDialog::onCameraDisconnected()
{
    m_labelStatus->setText(tr("Disonnected"));
    m_labelStatus->setStyleSheet("color: rgb(255, 0, 0)");
}

void CameraAlignmentDialog::onMarkIndexChanged(bool checked)
{
    if (!checked)
        return;

    QRadioButton* button = qobject_cast<QRadioButton*>(sender());
    if (!button)
        return;

    int index = button->property("markIndex").toInt();
    m_currentMarkIndex = index;
}

void CameraAlignmentDialog::onDeviceStateChanged(DeviceState state)
{
    if (state.workingMode == LaserWorkMode::LWM_STOP)
    {
        QVector4D userOrigin = LaserApplication::device->userOrigin();
        qLogD << "user origin: " << userOrigin;
        LaserApplication::device->moveTo(userOrigin);
    }
}
