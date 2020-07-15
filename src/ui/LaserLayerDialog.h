#ifndef LASERLAYERDIALOG_H
#define LASERLAYERDIALOG_H

#include "common/common.h"

#include <QDialog>
#include <QScopedPointer>

#include "scene/LaserLayer.h"

class QCloseEvent;

QT_BEGIN_NAMESPACE
namespace Ui { class LaserLayerDialog; }
QT_END_NAMESPACE

class LaserLayerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LaserLayerDialog(const QString& id, LaserLayer::LayerType type, QWidget* parent = nullptr);
    explicit LaserLayerDialog(const LaserLayer& layer, QWidget* parent = nullptr);
    ~LaserLayerDialog();

    LaserLayer layer() const { return m_layer; }

private:
    void initUi(bool editing);

protected:

protected slots:
    virtual void accept();

    void onLayerNameChanged(const QString& name);
    void onMinSpeedChanged(int value);
    void onRunSpeedChanged(int value);
    void onLaserPowerChanged(int value);
    void onEngravingForwardChanged(int state);
    void onEngravingStyleChanged(bool checked);
    void onLineSpacingChanged(int value);
    void onColumnSpacingChanged(int value);
    void onStartXChanged(int value);
    void onStartYChanged(int value);
    void onErrorXChanged(int value);
    void onMoveSpeedChanged(int value);
    void onMinSpeedPowerChanged(int value);
    void onRunSpeedPowerChanged(int value);

private:
    QScopedPointer<Ui::LaserLayerDialog> m_ui;
    LaserLayer m_layer;
};

#endif // LASERLAYERDIALOG_H