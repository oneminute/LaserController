#ifndef STAMSTRIPPDIALOG_H
#define STAMSTRIPPDIALOG_H
#include<QStandardItem>
#include <QDialog>
#include <QScopedPointer>
#include<scene/LaserScene.h>
#include <widget/LaserViewer.h>
#include "ui/StampDialog.h"
namespace Ui
{
    class StampStripDialog;
}
class StampStripDialog : public StampDialog {
    Q_OBJECT
public:
    StampStripDialog(LaserScene* scene, QWidget* parent = nullptr);
    virtual ~StampStripDialog();
    void rectifyTextSize(qreal w, qreal h, LaserHorizontalText* text);
    void rectifyTextSize(qreal w, qreal h, LaserVerticalText* text);
    QList<LaserPrimitive*> createStampPrimitive();
private:
    QScopedPointer<Ui::StampStripDialog> m_ui;
    QStandardItemModel* m_viewItemModel;
    int m_preLayoutIndex;
    QList<QMap<QModelIndex, QString>> m_tablesModelList;
    int m_layerIndex;
    QString m_defaultTexts[3] = {tr("single-line"), tr("multi-row"), tr("multi-column")};

};
#endif // STAMPDIALOG_H
