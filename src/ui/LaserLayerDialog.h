#ifndef LASERLAYERDIALOG_H
#define LASERLAYERDIALOG_H

#include "common/common.h"

#include <QDialog>
#include <QScopedPointer>

class QCloseEvent;
class LaserLayer;
class LaserDocument;
class InputWidgetWrapper;
class QAbstractButton;

QT_BEGIN_NAMESPACE
namespace Ui { class LaserLayerDialog; }
QT_END_NAMESPACE

class LaserLayerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LaserLayerDialog(LaserLayer* layer, QWidget* parent = nullptr);
    ~LaserLayerDialog();

    LaserLayer* layer() const { return m_layer; }

private:
    void initUi();

    void resetParameters();

	void restoreParameters();

protected:

protected slots:
    virtual void accept();

    void onCuttingToggled(bool checked);
    void onEngravingToggled(bool checked);
    void onFillingToggled(bool checked);
    void onStampToggled(bool checked);
    void onEngravingEnableCuttingToggled(bool checked);
    void onFillingEnableCuttingToggled(bool checked);
    void onFillingTypeChanged(int index);

	void onButtonClicked(QAbstractButton *button);

    void updateControls();

private:
    QScopedPointer<Ui::LaserLayerDialog> m_ui;
    LaserDocument* m_doc;
    LaserLayer* m_layer;
    LaserLayerType m_type;
    QList<InputWidgetWrapper*> m_wrappers;
};

#endif // LASERLAYERDIALOG_H