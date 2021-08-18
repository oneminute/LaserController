#ifndef SMALLDIAGONALLIMITATIONWIDGET_H
#define SMALLDIAGONALLIMITATIONWIDGET_H

#include <QObject>
#include <QWidget>
#include <QGridLayout>

#include "scene/SmallDiagonalLimitation.h"

class SmallDiagonalLimitationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SmallDiagonalLimitationWidget(QWidget* parent = nullptr);
    SmallDiagonalLimitationWidget(const SmallDiagonalLimitation& limitation, QWidget* parent = nullptr);
    ~SmallDiagonalLimitationWidget() {}

protected:
    void init();
    void addRow(int row, const SmallDiagonalLimitationItem& item);

protected slots:
    void onAddButtonClicked(bool checked = false);

private:
    QGridLayout* m_layout;
    SmallDiagonalLimitation m_limitation;

    Q_DISABLE_COPY(SmallDiagonalLimitationWidget)
};

#endif // SMALLDIAGONALLIMITATIONWIDGET_H
