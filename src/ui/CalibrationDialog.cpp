#include "CalibrationDialog.h"

#include <QCheckBox>
#include <QLabel>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QPainter>

#include "common/Config.h"
#include "camera/DistortionCalibrator.h"
#include "widget/ImageViewer.h"
#include "widget/InputWidgetWrapper.h"

CalibrationDialog::CalibrationDialog(CameraController* cameraController, DistortionCalibrator* calibrator, QWidget* parent)
    : WizardDialog(parent)
    , m_cameraController(cameraController)
    , m_calibrator(calibrator)
    , m_requestCalibration(false)
{
    m_labelStatus = new QLabel;
    m_labelStatus->setText(tr("Disconnected"));
    m_frameStatus = new QLabel;
    QHBoxLayout* layout = buttonsLayout();
    layout->insertStretch(0);
    layout->insertWidget(0, m_labelStatus);
    layout->insertWidget(1, m_frameStatus);

    m_page1 = new WizardDialogPage(tr("Introduce"));
    m_page1Introduction = new QLabel();
    m_page1Introduction->setWordWrap(true);
    m_page1Introduction->setText(
        tr(
            "<h1>Introduction</h1>\n"
            "<ol>"
            "<li>Please find the usb cable on the machine and plug it into the usb slot of the computer;</li>"
            "<li>Confirm that the fill light can work normally;</li>"
            "<li>If it is the first time to use, you may need to manually adjust the focal length of the lens, the next page will guide you to do so;</li>"
            "<li>This wizard will guide you through the lens distortion correction operation, please do not disconnect the camera during the operation.</li>"
            "</ol>"
        )
    );
    QVBoxLayout* page1Layout = new QVBoxLayout;
    page1Layout->addWidget(m_page1Introduction);
    page1Layout->addStretch(1);
    m_page1->setLayout(page1Layout);
    appendPage(m_page1);

    m_page2 = new WizardDialogPage(tr("Ajust Focal"));
    QVBoxLayout* page2Layout = new QVBoxLayout;
    m_page2Introduction = new QLabel;
    m_page2Introduction->setWordWrap(true);
    m_page2Introduction->setText(
        tr(
            "<h1>Adjust Focal</h1>"
            "<ol>"
            "<li>Please place the attached lens calibration board (shown in the left 1 figure below) in front of the lens.</li>"
            "<li>Rotate the camera lens as shown in the figure on the left 2 below, and after seeing a clear picture in the figure below on the right, the focus adjustment is completed.</li>"
            "</ol>"
        )
    );
    QFormLayout* page2SettingsLayout = new QFormLayout;
    QCheckBox* page2Fisheye = InputWidgetWrapper::createWidget<QCheckBox*>(Config::Camera::fisheyeItem());
    Config::Camera::fisheyeItem()->bindWidget(page2Fisheye, SS_DIRECTLY);
    QComboBox* page2Resolutions = InputWidgetWrapper::createWidget<QComboBox*>(Config::Camera::resolutionItem());
    Config::Camera::resolutionItem()->bindWidget(page2Resolutions, SS_DIRECTLY);
    QPushButton* page2ButtonRestart = new QPushButton(tr("Restart"));
    connect(page2ButtonRestart, &QPushButton::clicked, [=]()
        {
            m_cameraController->restart();
        }
    );
    page2SettingsLayout->addRow(
        Config::Camera::fisheyeItem()->title(),
        page2Fisheye
    );
    page2SettingsLayout->addRow(
        Config::Camera::resolutionItem()->title(),
        page2Resolutions
    );
    page2SettingsLayout->addWidget(page2ButtonRestart);
    QHBoxLayout* page2TitleLayout = new QHBoxLayout;
    page2TitleLayout->addWidget(m_page2Introduction);
    page2TitleLayout->addLayout(page2SettingsLayout);
    page2TitleLayout->setStretch(0, 1);
    page2TitleLayout->setStretch(1, 0);
    page2Layout->addLayout(page2TitleLayout);
    QGridLayout* page2ImagesLayout = new QGridLayout;
    
    QLabel* page2AdjustFocal = new QLabel;
    page2AdjustFocal->setPixmap(QPixmap(":/ui/icons/images/adjust_focal.png").scaled(200, 200, Qt::KeepAspectRatio));
    QLabel* page2CalibrationBoard = new QLabel;
    page2CalibrationBoard->setPixmap(QPixmap(":/ui/icons/images/calibration_chessboard.png").scaled(200, 200, Qt::KeepAspectRatio));
    m_page2ImageViewer = new ImageViewer;
    page2ImagesLayout->addWidget(page2AdjustFocal, 0, 0);
    page2ImagesLayout->addWidget(page2CalibrationBoard, 1, 0);
    page2ImagesLayout->addWidget(m_page2ImageViewer, 0, 1, 2, 1);
    page2ImagesLayout->setColumnStretch(0, 0);
    page2ImagesLayout->setColumnStretch(1, 1);
    page2ImagesLayout->setRowStretch(0, 1);
    page2ImagesLayout->setRowStretch(1, 1);
    page2Layout->addLayout(page2ImagesLayout);
    page2Layout->setStretch(0, 0);
    page2Layout->setStretch(1, 1);
    m_page2->setLayout(page2Layout);
    appendPage(m_page2);

    m_page3 = new WizardDialogPage(tr("Capture"));
    QVBoxLayout* page3Layout = new QVBoxLayout;
    QLabel* page3Guide = new QLabel;
    page3Guide->setWordWrap(true);
    page3Guide->setText(tr(
        "<h1>Capture Images</h1>"
        "<ol>"
        "<li>Please follow the picture requirements to ensure that the calibration plate faces the camera lens as vertically as possible, and keep a distance of about 15 to 20 cm between the calibration plate and the lens;</li>"
        "<li>If you check the automatic capture, the user needs to hold the calibration plate and move it smoothly in front of the camera lens, as far as possible to cover the entire camera viewing area;</li>"
        "<li>In the frame list below, there is a list of captured calibration pictures with corresponding quality scores. You can independently consider whether to adopt a certain calibration picture;</li>"
        "<li>When you are ready, click the start capture button below.</li>"
        "</ol>"
    ));
    QLabel* page3GuideImage = new QLabel;
    page3GuideImage->setPixmap(QPixmap(":/ui/icons/images/calibration_guide.png").scaled(QSize(200, 200), Qt::KeepAspectRatio));
    QFormLayout* page3FormLayout = new QFormLayout;
    m_page3SamplesCount = new QLabel(tr("0"));
    m_page3Coverage = new QLabel(tr("0%"));
    m_page3Scores = new QLabel(tr("0"));
    page3FormLayout->addRow(tr("Samples"), m_page3SamplesCount);
    page3FormLayout->addRow(tr("Converage"), m_page3Coverage);
    page3FormLayout->addRow(tr("Scores"), m_page3Scores);
    //m_page3SamplesTable = new QTableWidget;
    //m_page3AutoCapture = new QCheckBox(tr("Auto Capture"));
    QCheckBox* page3AutoCapture = InputWidgetWrapper::createWidget<QCheckBox*>(Config::Camera::calibrationAutoCaptureItem());
    m_page3ButtonCapture = new QPushButton(tr("Capture"));
    connect(m_page3ButtonCapture, &QPushButton::clicked, this, &CalibrationDialog::onButtonCaptureClicked);
    m_page3ButtonStart = new QPushButton(tr("Start"));
    m_page3ButtonStop = new QPushButton(tr("Stop"));
    m_page3ButtonDelete = new QPushButton(tr("Delete"));
    connect(m_page3ButtonDelete, &QPushButton::clicked, this, &CalibrationDialog::removeCurrentSample);
#ifdef _DEBUG
    m_page3ButtonLoadSamples = new QPushButton(tr("Load"));
    connect(m_page3ButtonLoadSamples, &QPushButton::clicked, this, &CalibrationDialog::loadSamples);
    m_page3ButtonSaveSamples = new QPushButton(tr("Save"));
    connect(m_page3ButtonSaveSamples, &QPushButton::clicked, this, &CalibrationDialog::saveSamples);
#endif
    QVBoxLayout* page3ButtonsLayout = new QVBoxLayout;
    page3ButtonsLayout->addLayout(page3FormLayout);
    //page3ButtonsLayout->addWidget(page3AutoCapture);
    page3ButtonsLayout->addWidget(m_page3ButtonCapture);
    //page3ButtonsLayout->addWidget(m_page3ButtonStart);
    //page3ButtonsLayout->addWidget(m_page3ButtonStop);
    page3ButtonsLayout->addWidget(m_page3ButtonDelete);
#ifdef _DEBUG
    page3ButtonsLayout->addWidget(m_page3ButtonLoadSamples);
    page3ButtonsLayout->addWidget(m_page3ButtonSaveSamples);
#endif
    //imagesLayout->addLayout(page3ButtonsLayout, 0, 1, 2, 1);
    QHBoxLayout* page3GuideLayout = new QHBoxLayout;
    page3GuideLayout->addWidget(page3GuideImage);
    page3GuideLayout->addWidget(page3Guide);
    page3GuideLayout->addLayout(page3ButtonsLayout);
    page3GuideLayout->setStretch(0, 0);
    page3GuideLayout->setStretch(1, 1);
    page3GuideLayout->setStretch(2, 0);
    m_page3ImageViewer = new ImageViewer;
    m_page3ImageViewerSample = new ImageViewer;
    QGridLayout* imagesLayout = new QGridLayout;
    imagesLayout->addWidget(m_page3ImageViewer, 0, 0);
    imagesLayout->addWidget(m_page3ImageViewerSample, 0, 1);
    imagesLayout->setColumnStretch(0, 1);
    imagesLayout->setColumnStretch(1, 1);
    page3Layout->addLayout(page3GuideLayout);
    page3Layout->addLayout(imagesLayout);
    //page3Layout->addLayout(page3ConfigLayout);
    page3Layout->setStretch(0, 0);
    page3Layout->setStretch(1, 1);
    page3Layout->setStretch(2, 0);
    m_page3->setLayout(page3Layout);
    appendPage(m_page3);

    m_page4 = new WizardDialogPage(tr("Calibration"));
    QVBoxLayout* page4Layout = new QVBoxLayout;
    m_page4Introduction = new QLabel;
    m_page4Introduction->setText(tr(
        "<h1>Calibration</h1>"
        "<ol>"
        "<li>Click the \"calibrate\" button to do calibration;</li>"
        "<li>When it has been done, you will see the undistorted image below;</li>"
        "</ol>"
    ));
    m_page4ImageViewer = new ImageViewer;
    QHBoxLayout* page4ButtonsLayout = new QHBoxLayout;
    m_page4ButtonVerify = new QPushButton;
    m_page4ButtonVerify->setText(tr("Verify"));
    m_page4ButtonSave = new QPushButton;
    m_page4ButtonSave->setText(tr("Save"));
    page4ButtonsLayout->addWidget(m_page4ButtonVerify);
    page4ButtonsLayout->addWidget(m_page4ButtonSave);
    connect(m_page4ButtonSave, &QPushButton::clicked, this, &CalibrationDialog::onButtonSaveCalibrationClicked);
    connect(m_page4ButtonVerify, &QPushButton::clicked, this, &CalibrationDialog::onButtonVerifyCalibrationClicked);
    page4Layout->addWidget(m_page4Introduction);
    page4Layout->addWidget(m_page4ImageViewer);
    page4Layout->addLayout(page4ButtonsLayout);
    m_page4->setLayout(page4Layout);
    appendPage(m_page4);

    connect(m_page1, &WizardDialogPage::entered, this, &CalibrationDialog::onPage1Entered);
    connect(m_page1, &WizardDialogPage::exited, this, &CalibrationDialog::onPage1Exited);
    connect(m_page2, &WizardDialogPage::entered, this, &CalibrationDialog::onPage2Entered);
    connect(m_page2, &WizardDialogPage::exited, this, &CalibrationDialog::onPage2Exited);
    connect(m_page3, &WizardDialogPage::entered, this, &CalibrationDialog::onPage3Entered);
    connect(m_page3, &WizardDialogPage::exited, this, &CalibrationDialog::onPage3Exited);
    connect(m_page4, &WizardDialogPage::entered, this, &CalibrationDialog::onPage4Entered);
    connect(m_page4, &WizardDialogPage::exited, this, &CalibrationDialog::onPage4Exited);

    setLeftLayoutWidth(120);

    connect(m_cameraController, &CameraController::connected, this, &CalibrationDialog::onCameraConnected);
    connect(m_cameraController, &CameraController::disconnected, this, &CalibrationDialog::onCameraDisconnected);
    m_cameraController->start();

    updatePage();

    resize(1200, 800);
    onCameraConnected();

    installEventFilter(this);
    m_cameraController->registerSubscriber(this);
}

CalibrationDialog::~CalibrationDialog()
{
    m_cameraController->unregisterSubscriber(this);
}

bool CalibrationDialog::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == Event_CameraFrame) {
        CameraFrameEvent* frameEvent = static_cast<CameraFrameEvent*>(event);
        m_frameStatus->setText(tr("fps: %1, duration: %2").arg(1000.0 / frameEvent->duration(), 0, 'f', 3).arg(frameEvent->duration()));
        if (currentIndex() == 1)
        {
            QImage image(frameEvent->thumbImage().data, frameEvent->thumbImage().cols, frameEvent->thumbImage().rows, frameEvent->thumbImage().step, QImage::Format_RGB888);
            QSize imageSize = image.size();
            QPainter painter;
            painter.begin(&image);
            painter.setPen(QPen(Qt::blue, 1, Qt::SolidLine));
            QPoint center((imageSize.width() - 1) / 2, (imageSize.height() - 1) / 2);
            int radius = 50;
            painter.drawLine(center + QPoint(-radius, 0), center + QPoint(radius, 0));
            painter.drawLine(center + QPoint(0, -radius), center + QPoint(0, radius));
            painter.end();
            m_page2ImageViewer->setImage(image);
        }
        else if (currentIndex() == 2)
        {
            QImage image(frameEvent->thumbImage().data, frameEvent->thumbImage().cols, frameEvent->thumbImage().rows, frameEvent->thumbImage().step, QImage::Format_RGB888);
            QSize imageSize = image.size();
            QPainter painter;
            painter.begin(&image);
            painter.setPen(QPen(Qt::blue, 1, Qt::SolidLine));
            for (int r = 1; r < 3; r++)
            {
                int y = imageSize.height() * r / 3;
                painter.drawLine(QPoint(0, y), QPoint(imageSize.width(), y));
            }
            for (int c = 1; c < 3; c++)
            {
                int x = imageSize.width() * c / 3;
                painter.drawLine(QPoint(x, 0), QPoint(x, imageSize.height()));
            }
            painter.end();
            m_page3ImageViewer->setImage(image);

            if (m_requestCalibration)
            {
                m_requestCalibration = false;

                qreal error = m_calibrator->captureSample(frameEvent->originImage(), true);
                m_page3Scores->setText(tr("%1").arg(error));
                QImage originImage(frameEvent->originImage().data, frameEvent->originImage().cols, frameEvent->originImage().rows, frameEvent->originImage().step, QImage::Format_RGB888);
                m_page3ImageViewerSample->setImage(originImage);
                m_page3SamplesCount->setText(tr("%1").arg(m_calibrator->calibrationSamplesCount()));
            }
        }
        else if (currentIndex() == 3)
        {
            if (m_requestCalibration)
            {
                m_requestCalibration = false;

                qreal error = m_calibrator->undistortImage(frameEvent->originImage());
                QImage image(frameEvent->originImage().data, frameEvent->originImage().cols, frameEvent->originImage().rows, frameEvent->originImage().step, QImage::Format_RGB888);

                QSize imageSize = image.size();
                QPainter painter;
                painter.begin(&image);
                painter.setPen(QPen(Qt::blue, 1, Qt::SolidLine));
                for (int r = 1; r < 4; r++)
                {
                    int y = imageSize.height() * r / 4;
                    painter.drawLine(QPoint(0, y), QPoint(imageSize.width(), y));
                }
                for (int c = 1; c < 8; c++)
                {
                    int x = imageSize.width() * c / 8;
                    painter.drawLine(QPoint(x, 0), QPoint(x, imageSize.height()));
                }
                painter.end();
                m_page4ImageViewer->setImage(image);
            }
        }
        return true;
    }
    else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}

void CalibrationDialog::onPage1Entered()
{
}

void CalibrationDialog::onPage1Exited()
{
}

void CalibrationDialog::onPage2Entered()
{
    m_page2ImageViewer->fitBy(Config::Camera::thumbResolution());
}

void CalibrationDialog::onPage2Exited()
{
}

void CalibrationDialog::onPage3Entered()
{
    m_calibrator->validate();
    m_page3ImageViewer->fitBy(Config::Camera::thumbResolution());
    m_page3ImageViewerSample->fitBy(Config::Camera::resolution());
}

void CalibrationDialog::onPage3Exited()
{
}

void CalibrationDialog::onPage4Entered()
{
    m_page4ImageViewer->fitBy(Config::Camera::resolution());
}

void CalibrationDialog::onPage4Exited()
{
}

void CalibrationDialog::onCameraConnected()
{
    m_labelStatus->setText(tr("Connected"));
    m_labelStatus->setStyleSheet("color: rgb(0, 255, 0)");
}

void CalibrationDialog::onCameraDisconnected()
{
    m_labelStatus->setText(tr("Disonnected"));
    m_labelStatus->setStyleSheet("color: rgb(255, 0, 0)");
}

void CalibrationDialog::onButtonCaptureClicked()
{
    m_requestCalibration = true;
}

void CalibrationDialog::updatePage3Buttons(const QVariant& autoCapture)
{
    if (Config::Camera::calibrationAutoCapture())
    {
        m_page3ButtonCapture->setEnabled(false);
        m_page3ButtonStart->setEnabled(true);
        m_page3ButtonStop->setEnabled(true);
    }
    else
    {
        m_page3ButtonCapture->setEnabled(true);
        m_page3ButtonStart->setEnabled(false);
        m_page3ButtonStop->setEnabled(false);
    }
}

void CalibrationDialog::loadSamples()
{
    m_calibrator->loadSamples();
}

void CalibrationDialog::saveSamples()
{
    m_calibrator->saveSamples();
}

void CalibrationDialog::removeCurrentSample()
{
    m_calibrator->removeCurrentItem();
    m_page3SamplesCount->setText(tr("%1").arg(m_calibrator->calibrationSamplesCount()));
}

void CalibrationDialog::onButtonSaveCalibrationClicked(bool checked)
{
    m_calibrator->saveCoeffs();
}

void CalibrationDialog::onButtonVerifyCalibrationClicked(bool checked)
{
    m_requestCalibration = true;
}
