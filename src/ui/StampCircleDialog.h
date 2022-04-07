#ifndef STAMPCIRCLEDIALOG_H
#define STAMPCIRCLEDIALOG_H
#include <QDialog>
#include<QStandardItem>
#include <QScopedPointer>
#include "ui/StampDialog.h"
#include "scene/LaserScene.h"
#include "widget/LaserViewer.h"
namespace Ui
{
    class StampCircleDialog;
}
class StampCircleDialog : public StampDialog {
    Q_OBJECT
public:
    StampCircleDialog(LaserScene* scene, bool isEllipse = false,  QWidget* parent = nullptr);
    virtual ~StampCircleDialog();
    void addTableViewRow(int row, QString contentStr, QString fontStr, 
        QString propertyStr,Qt::CheckState checkState, qreal textSpacing, qreal textHeight);
    QList<LaserPrimitive*> createStampPrimitive();
private:
    typedef struct itemStruct {
    private:
        QString str;
        Qt::CheckState checkState;
    public:
        itemStruct(QString _str, Qt::CheckState _checked) {
            str = _str;
            checkState = _checked;
        }
        QString getStr() { return str; }
        Qt::CheckState getCheckState() { return checkState; }
    } _itemStruct;
    QScopedPointer<Ui::StampCircleDialog> m_ui;
    QStandardItemModel* m_viewItemModel;
    int m_preLayoutIndex;
    QString m_checkedItemStyle;
    QString m_uncheckedItemStyle;
    QList<QMap<QModelIndex, itemStruct>> m_tablesModelList;
    QString m_textRowProperty[5] = {
        QString(tr("Top Circle Text")), QString(tr("Horizontal Text")), QString(tr("Bottom Circle Text")),
        QString(tr("Horizontal Invoice Number")), QString(tr("Bottom Horizontal Number"))
    };
    QString m_textInitRowContent[5] = {
        QString(tr("Laser Test Seal")), QString(tr("Horizontal Text")), QString(tr("123456789012345678")), 
        QString(tr("Invoice Seal")), QString("(0)")
    };
    bool m_isEllipse;
    int m_layerIndex;
protected slots:
    virtual void accept();
};
#endif // STAMPDIALOG_H
