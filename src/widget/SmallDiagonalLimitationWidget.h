#ifndef SMALLDIAGONALLIMITATIONWIDGET_H
#define SMALLDIAGONALLIMITATIONWIDGET_H

#include <QObject>
#include <QWidget>
#include <QGridLayout>

#include "common/ConfigItem.h"
#include "scene/SmallDiagonalLimitation.h"

class SmallDiagonalLimitationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SmallDiagonalLimitationWidget(QWidget* parent = nullptr);
    SmallDiagonalLimitationWidget(ConfigItem* item, QWidget* parent = nullptr);
    ~SmallDiagonalLimitationWidget() {}

protected:
    void init();
    void updateLimitations();
    void addRow(int row, SmallDiagonalLimitationItem* item);
    void removeRow(int row);
    void emitValueChanged();
    void clearWidgets();

protected slots:
    void onAddButtonClicked(bool checked = false);

signals:
    void valueChanged(SmallDiagonalLimitation* value);

private:
    QVBoxLayout* m_layout;
    QGridLayout* m_gridLayout;
    ConfigItem* m_item;
    SmallDiagonalLimitation* m_limitation;

    Q_DISABLE_COPY(SmallDiagonalLimitationWidget)
};

#endif // SMALLDIAGONALLIMITATIONWIDGET_H
