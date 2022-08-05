#ifndef STAMFRAMEPDIALOG_H
#define STAMPFRAMEDIALOG_H

#include <QDialog>
#include <QScopedPointer>
#include "ui/StampDialog.h"

namespace Ui
{
    class StampFrameDialog;
}
class StampFrameDialog : public StampDialog {
    Q_OBJECT
public:
    StampFrameDialog(LaserScene* scene, LaserLayer* layer, QWidget* parent = nullptr);
    virtual ~StampFrameDialog();
    virtual QList<LaserPrimitive*> createStampPrimitive();
private:
    QScopedPointer<Ui::StampFrameDialog> m_ui;
    LaserScene* m_scene;
    LaserViewer* m_viewer;
    QString m_defaultTexts[3] = {tr("FSNS"), tr("FSN"), tr("NA") };//姓氏名印，姓氏名，姓名

};
#endif // STAMPDIALOG_H
