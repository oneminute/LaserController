#ifndef MULTIDUPLICATIONDIALOG_H
#define MULTIDUPLICATIONDIALOG_H

#include <QDialog>
#include <QWidget>
class LaserViewer;
class LaserPrimitiveGroup;
namespace Ui
{
    class MultiDuplicationDialog;
}
class MultiDuplicationDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MultiDuplicationDialog(LaserViewer* view, 
        int copies, int hSettingsVal, int vSettingsVal, int hDirectionVal, int vDirectionVal,
        qreal hDistanceLastVal, qreal vDistanceLastVal,
        QWidget* parent = nullptr);
    ~MultiDuplicationDialog();

    void HorizontalComboxChanged(int index);
    void VerticalComboxChanged(int index);
    virtual void accept();

    int HSettingsVal();
    int VSettingsVal();
    int HDirectionVal();
    int VDirectionVal();
    double HDistanceVal();
    double VDistanceVal();
    int copiesVal();
private:
    QScopedPointer<Ui::MultiDuplicationDialog> m_ui;
    LaserViewer* m_viewer;
    LaserPrimitiveGroup* m_group;

    int m_copiesVal;
    int m_HSettingsVal;
    int m_VSettingsVal;
    int m_HDirectionVal;
    int m_VDirectionVal;
    qreal m_HDistanceVal;
    qreal m_VDistanceVal;
};
#endif //MULTIDUPLICATIONDIALOG_H