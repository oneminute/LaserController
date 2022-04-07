#ifndef STAMFRAMEPDIALOG_H
#define STAMPFRAMEDIALOG_H
#include <QDialog>
#include <QScopedPointer>
#include<scene/LaserScene.h>
#include <widget/LaserViewer.h>
#include "ui/StampDialog.h"
namespace Ui
{
    class StampFrameDialog;
}
class StampFrameDialog : public StampDialog {
    Q_OBJECT
public:
    StampFrameDialog(LaserScene* scene, QWidget* parent = nullptr);
    virtual ~StampFrameDialog();
    QList<LaserPrimitive*> createStampPrimitive();
private:
    QScopedPointer<Ui::StampFrameDialog> m_ui;
    LaserScene* m_scene;
    LaserViewer* m_viewer;
    int m_layerIndex;
    QString m_defaultTexts[3] = {tr("FSNS"), tr("FSN"), tr("NA") };//–’ œ√˚”°£¨–’ œ√˚£¨–’√˚

};
#endif // STAMPDIALOG_H
