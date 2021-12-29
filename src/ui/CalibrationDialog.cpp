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

CalibrationDialog::CalibrationDialog(QWidget* parent)
    : WizardDialog(parent)
{
    m_labelStatus = new QLabel;
    m_labelStatus->setText(tr("Disconnected"));
    QHBoxLayout* layout = buttonsLayout();
    layout->insertStretch(0);
    layout->insertWidget(0, m_labelStatus);

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
    page2Layout->addWidget(m_page2Introduction);
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
    QHBoxLayout* page3GuideLayout = new QHBoxLayout;
    page3GuideLayout->addWidget(page3GuideImage);
    page3GuideLayout->addWidget(page3Guide);
    page3GuideLayout->setStretch(0, 0);
    page3GuideLayout->setStretch(1, 1);
    m_page3ImageViewer = new ImageViewer;
    m_page3ImageViewerSample = new ImageViewer;
    QHBoxLayout* imagesLayout = new QHBoxLayout;
    imagesLayout->addWidget(m_page3ImageViewer);
    imagesLayout->addWidget(m_page3ImageViewerSample);
    QFormLayout* page3FormLayout = new QFormLayout;
    m_page3SamplesCount = new QLabel;
    m_page3Coverage = new QLabel;
    page3FormLayout->addRow(tr("Samples"), m_page3SamplesCount);
    page3FormLayout->addRow(tr("Converage"), m_page3Coverage);
    m_page3SamplesTable = new QTableWidget;
    //m_page3AutoCapture = new QCheckBox(tr("Auto Capture"));
    QCheckBox* page3AutoCapture = InputWidgetWrapper::createWidget<QCheckBox*>(Config::Camera::calibrationAutoCaptureItem());
    m_page3ButtonCapture = new QPushButton(tr("Capture"));
    connect(m_page3ButtonCapture, &QPushButton::clicked, this, &CalibrationDialog::onButtonCaptureClicked);
    m_page3ButtonStart = new QPushButton(tr("Start"));
    m_page3ButtonStop = new QPushButton(tr("Stop"));
    m_page3ButtonDelete = new QPushButton(tr("Delete"));
#ifdef _DEBUG
    m_page3ButtonLoadSamples = new QPushButton(tr("Load"));
    connect(m_page3ButtonLoadSamples, &QPushButton::clicked, this, &CalibrationDialog::loadSamples);
    m_page3ButtonSaveSamples = new QPushButton(tr("Save"));
    connect(m_page3ButtonSaveSamples, &QPushButton::clicked, this, &CalibrationDialog::saveSamples);
#endif
    QVBoxLayout* page3ButtonsLayout = new QVBoxLayout;
    page3ButtonsLayout->addWidget(page3AutoCapture);
    page3ButtonsLayout->addWidget(m_page3ButtonCapture);
    page3ButtonsLayout->addWidget(m_page3ButtonStart);
    page3ButtonsLayout->addWidget(m_page3ButtonStop);
    page3ButtonsLayout->addWidget(m_page3ButtonDelete);
#ifdef _DEBUG
    page3ButtonsLayout->addWidget(m_page3ButtonLoadSamples);
    page3ButtonsLayout->addWidget(m_page3ButtonSaveSamples);
#endif
    QHBoxLayout* page3ConfigLayout = new QHBoxLayout;
    page3ConfigLayout->addLayout(page3FormLayout);
    page3ConfigLayout->addWidget(m_page3SamplesTable);
    page3ConfigLayout->addLayout(page3ButtonsLayout);
    page3ConfigLayout->setStretch(0, 1);
    page3ConfigLayout->setStretch(1, 1);
    page3ConfigLayout->setStretch(2, 1);
    page3Layout->addLayout(page3GuideLayout);
    page3Layout->addLayout(imagesLayout);
    page3Layout->addLayout(page3ConfigLayout);
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
    m_page4ButtonCalibrate = new QPushButton;
    m_page4ButtonCalibrate->setText(tr("Calibrate"));
    m_page4ButtonSave = new QPushButton;
    m_page4ButtonSave->setText(tr("Save"));
    page4ButtonsLayout->addWidget(m_page4ButtonCalibrate);
    connect(m_page4ButtonCalibrate, &QPushButton::clicked, this, &CalibrationDialog::onButtonCalibrateClicked);
    page4ButtonsLayout->addWidget(m_page4ButtonSave);
    connect(m_page4ButtonSave, &QPushButton::clicked, this, &CalibrationDialog::onButtonSaveCalibrationClicked);
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

    m_cameraController = new CameraController;
    m_calibrator = new DistortionCalibrator;
    m_cameraController->installProcessor(m_calibrator);
    connect(m_cameraController, &CameraController::connected, this, &CalibrationDialog::onCameraConnected);
    connect(m_cameraController, &CameraController::disconnected, this, &CalibrationDialog::onCameraDisconnected);
    connect(m_cameraController, &CameraController::frameCaptured, this, &CalibrationDialog::onFrameCaptured);
    connect(m_calibrator, &DistortionCalibrator::sampleCaptured, this, &CalibrationDialog::onCalibrationSampleCaptured);
    connect(m_calibrator, &DistortionCalibrator::calibrated, this, &CalibrationDialog::onCalibrated);
    m_cameraController->start();

    updatePage();

    resize(1200, 800);
}

CalibrationDialog::~CalibrationDialog()
{
    m_cameraController->uninstallProcessor(m_calibrator);
    SAFE_DELETE(m_cameraController);
    SAFE_DELETE(m_calibrator);
}

void CalibrationDialog::onPage1Entered()
{
    m_calibrator->setEnabled(false);
}

void CalibrationDialog::onPage1Exited()
{
}

void CalibrationDialog::onPage2Entered()
{
    m_calibrator->setEnabled(true);
    m_calibrator->setRole(DistortionCalibrator::Role_Idle);
    m_page2ImageViewer->fitBy(Config::Camera::resolution());
}

void CalibrationDialog::onPage2Exited()
{
}

void CalibrationDialog::onPage3Entered()
{
    m_calibrator->setEnabled(true);
    m_calibrator->setRole(DistortionCalibrator::Role_Capture);
    m_page3ImageViewer->fitBy(Config::Camera::resolution());
    m_page3ImageViewerSample->fitBy(Config::Camera::resolution());
}

void CalibrationDialog::onPage3Exited()
{
}

void CalibrationDialog::onPage4Entered()
{
    m_calibrator->setEnabled(true);
    m_calibrator->setRole(DistortionCalibrator::Role_Undistortion);
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

void CalibrationDialog::onFrameCaptured()
{
    cv::Mat mat = m_cameraController->image();
    QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
    if (currentIndex() == 1)
    {
        m_page2ImageViewer->setImage(image);
    }
    else if (currentIndex() == 2)
    {
        m_page3ImageViewer->setImage(image);
    }
    else if (currentIndex() == 3)
    {
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

void CalibrationDialog::onCalibrationSampleCaptured()
{
    m_page3SamplesCount->setText(tr("%1/%2").arg(m_calibrator->calibrationSamplesCount()).arg(10));
}

void CalibrationDialog::onCalibrated()
{
    m_calibrator->setRole(DistortionCalibrator::Role_Undistortion);
}

void CalibrationDialog::onButtonCaptureClicked()
{
    m_calibrator->requestCapture();
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

void CalibrationDialog::onButtonCalibrateClicked(bool checked)
{
    m_calibrator->requestCalibration();
}

void CalibrationDialog::onButtonSaveCalibrationClicked(bool checked)
{
}
