#ifndef CAMERAALIGNMENTDIALOG_H
#define CAMERAALIGNMENTDIALOG_H

#include "WizardDialog.h"

#include <QMap>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/tracking/tracking_legacy.hpp>

#include "laser/LaserDefines.h"

class QSpinBox;
class QDoubleSpinBox;
class ImageViewer;
class LaserDocument;
class LaserLayer;
class QGraphicsScene;
class QGraphicsView;
class QGraphicsItem;
class DistortionCalibrator;
class GraphicsViewEx;

class CameraAlignmentDialog : public WizardDialog
{
    Q_OBJECT
public:
    explicit CameraAlignmentDialog(QWidget* parent = nullptr);
    ~CameraAlignmentDialog();

protected:
    virtual void closeEvent(QCloseEvent* e) override;

    QPainterPath quarterCircle(const QPoint& center, int radius);
    void setupLayers(LaserLayer* layer0, LaserLayer* layer1);

    void addMarkToScene(int halfMarkSize, const QPoint& center, const QPen& pen,
        QGraphicsScene* scene, QRect& rect, QPainterPath& path);
    void addMarkToDocument(int index, const QRect& rect, const QPainterPath& painterPath);

protected slots:
    void generate();
    void start();
    void stop();
    void capture();
    void calculate();
    void verify();
    void save();

    void setMarkPos(const QPointF& pos);

    void onCameraConnected();
    void onCameraDisconnected();
    void onFrameCaptured(cv::Mat processed, cv::Mat origin, FrameArgs args);
    void onMarkIndexChanged(bool checked);
    void onDeviceStateChanged(DeviceState state);

private:
    QLabel* m_labelStatus;
    QLabel* m_frameStatus;

    WizardDialogPage* m_page1;
    WizardDialogPage* m_page2;
    WizardDialogPage* m_page3;

    QLabel* m_page1Introduction;
    QSpinBox* m_page1SpinBoxHMargin;
    QSpinBox* m_page1SpinBoxVMargin;
    QSpinBox* m_page1SpinBoxMarkSize;
    QDoubleSpinBox* m_page1DoubleSpinBoxCuttingSpeed;
    QDoubleSpinBox* m_page1DoubleSpinBoxCuttingPower;
    QDoubleSpinBox* m_page1DoubleSpinBoxFillingSpeed;
    QDoubleSpinBox* m_page1DoubleSpinBoxFillingPower;
    QDoubleSpinBox* m_page1DoubleSpinBoxFillingInterval;
    QPushButton* m_page1ButtonGenerate;
    QPushButton* m_page1ButtonStart;
    QPushButton* m_page1ButtonStop;
    QGraphicsScene* m_page1Scene;
    QGraphicsView* m_page1Viewer;

    QLabel* m_page2Introduction;
    QGraphicsScene* m_page2Scene;
    GraphicsViewEx* m_page2Viewer;
    QPushButton* m_page2ButtonCapture;
    QPushButton* m_page2ButtonCalculate;

    QLabel* m_page3Introduction;
    QGraphicsScene* m_page3Scene;
    GraphicsViewEx* m_page3Viewer;
    QPushButton* m_page3ButtonVerify;
    QPushButton* m_page3ButtonSave;

    QPoint m_mark0;
    QPoint m_mark1;
    QPoint m_mark2;
    QPoint m_mark3;
    QPoint m_mark4;

    LaserDocument* m_doc;
    CameraController* m_cameraController;
    DistortionCalibrator* m_calibrator;

    bool m_waitingForImage;
    int m_currentMarkIndex;
    QMap<int, QGraphicsItem*> m_items;
    cv::Mat m_homography;
};

#endif // CAMERAALIGNMENTDIALOG_H