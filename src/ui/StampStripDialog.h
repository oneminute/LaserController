#ifndef STAMSTRIPPDIALOG_H
#define STAMSTRIPPDIALOG_H
#include <QStandardItem>
#include <QDialog>
#include <QScopedPointer>
#include "ui/StampDialog.h"

class LaserHorizontalText;
class LaserVerticalText;

namespace Ui
{
    class StampStripDialog;
}

class StampStripDialog : public StampDialog {
    Q_OBJECT
public:
    StampStripDialog(LaserScene* scene, LaserLayer* layer, QWidget* parent = nullptr);
    virtual ~StampStripDialog();
    void rectifyTextSize(qreal w, qreal h, LaserHorizontalText* text);
    void rectifyTextSize(qreal w, qreal h, LaserVerticalText* text);
    QList<LaserPrimitive*> createStampPrimitive();
private:
    QScopedPointer<Ui::StampStripDialog> m_ui;
    QStandardItemModel* m_viewItemModel;
    int m_preLayoutIndex;
    QList<QMap<QModelIndex, QString>> m_tablesModelList;
    QString m_defaultTexts[3] = {tr("single-line"), tr("multi-row"), tr("multi-column")};

};
#endif // STAMPDIALOG_H
