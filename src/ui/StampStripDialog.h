#ifndef STAMSTRIPPDIALOG_H
#define STAMSTRIPPDIALOG_H
#include<QStandardItem>
#include <QDialog>
#include <QScopedPointer>
#include<scene/LaserScene.h>
#include <widget/LaserViewer.h>
namespace Ui
{
    class StampStripDialog;
}
class StampStripDialog : public QDialog {
    Q_OBJECT
public:
    StampStripDialog(LaserScene* scene, QWidget* parent = nullptr);
    virtual ~StampStripDialog();
    void rectifyTextSize(qreal w, qreal h, LaserHorizontalText* text);
    void rectifyTextSize(qreal w, qreal h, LaserVerticalText* text);
private:
    QScopedPointer<Ui::StampStripDialog> m_ui;
    LaserScene* m_scene;
    LaserViewer* m_viewer;
    QStandardItemModel* m_viewItemModel;
    int m_preLayoutIndex;
    QList<QMap<QModelIndex, QString>> m_tablesModelList;
    
protected slots:
    virtual void accept();

};
#endif // STAMPDIALOG_H
