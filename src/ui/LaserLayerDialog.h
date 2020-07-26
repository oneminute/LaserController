#ifndef LASERLAYERDIALOG_H
#define LASERLAYERDIALOG_H

#include "common/common.h"

#include <QDialog>
#include <QScopedPointer>

class QCloseEvent;
class LaserLayer;

QT_BEGIN_NAMESPACE
namespace Ui { class LaserLayerDialog; }
QT_END_NAMESPACE

class LaserLayerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LaserLayerDialog(const QString& id, LayerType type, QWidget* parent = nullptr);
    explicit LaserLayerDialog(LaserLayer* layer, QWidget* parent = nullptr);
    ~LaserLayerDialog();

    LaserLayer* layer() const { return m_layer; }

private:
    void initUi(bool editing);

protected:

protected slots:
    virtual void accept();

private:
    QScopedPointer<Ui::LaserLayerDialog> m_ui;
    LaserLayer* m_layer;
};

#endif // LASERLAYERDIALOG_H