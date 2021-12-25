#ifndef CALIBRATIONDIALOG_H
#define CALIBRATIONDIALOG_H

#include "WizardDialog.h"

class ImageViewer;
class DistortionCalibrator;
class QCheckBox;
class QTableWidget;

class CalibrationDialog : public WizardDialog
{
    Q_OBJECT
public:
    explicit CalibrationDialog(QWidget* parent = nullptr);
    ~CalibrationDialog();

protected slots:
    void onPage2Entered();
    void onPage2Exited();

    void onCameraConnected();
    void onCameraDisconnected();

    void onFrameCaptured();

    void updatePage3Buttons(const QVariant& autoCapture);

private:
    QLabel* m_labelStatus;

    WizardDialogPage* m_page1;
    WizardDialogPage* m_page2;
    WizardDialogPage* m_page3;
    WizardDialogPage* m_page4;

    QLabel* m_page1Introduction;
    QLabel* m_page1Image1;
    QLabel* m_page2Introduction;
    ImageViewer* m_page2ImageViewer;
    ImageViewer* m_page3ImageViewer;
    ImageViewer* m_page3ImageViewerSample;
    QLabel* m_page3SamplesCount;
    QLabel* m_page3Coverage;
    QTableWidget* m_page3SamplesTable;
    //QCheckBox* m_page3AutoCapture;
    QPushButton* m_page3ButtonCapture;
    QPushButton* m_page3ButtonStart;
    QPushButton* m_page3ButtonStop;
    QPushButton* m_page3ButtonDelete;

    CameraController* m_cameraController;
    DistortionCalibrator* m_calibrator;
};

#endif // CALIBRATIONDIALOG_H