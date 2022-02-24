#ifndef STAMPCIRCLEDIALOG_H
#define STAMPCIRCLEDIALOG_H
#include <QDialog>
#include <QScopedPointer>
#include<scene/LaserScene.h>
#include <widget/LaserViewer.h>
namespace Ui
{
    class StampCircleDialog;
}
class StampCircleDialog : public QDialog {
    Q_OBJECT
public:
    StampCircleDialog(LaserScene* scene, QWidget* parent = nullptr);
    virtual ~StampCircleDialog();
private:
    QScopedPointer<Ui::StampCircleDialog> m_ui;
    LaserScene* m_scene;
    LaserViewer* m_viewer;
protected slots:
    virtual void accept();

};
#endif // STAMPDIALOG_H
