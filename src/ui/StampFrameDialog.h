#ifndef STAMFRAMEPDIALOG_H
#define STAMPFRAMEDIALOG_H
#include <QDialog>
#include <QScopedPointer>
#include<scene/LaserScene.h>
#include <widget/LaserViewer.h>
namespace Ui
{
    class StampFrameDialog;
}
class StampFrameDialog : public QDialog {
    Q_OBJECT
public:
    StampFrameDialog(LaserScene* scene, QWidget* parent = nullptr);
    virtual ~StampFrameDialog();
private:
    QScopedPointer<Ui::StampFrameDialog> m_ui;
    LaserScene* m_scene;
    LaserViewer* m_viewer;
protected slots:
    virtual void accept();

};
#endif // STAMPDIALOG_H