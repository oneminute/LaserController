#ifndef FINISHRUNWIDGET_H
#define FINISHRUNWIDGET_H

#include <QWidget>
#include "scene/LaserPrimitive.h"

class QCheckBox;

class FinishRunWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FinishRunWidget(QWidget* parent = nullptr);
    virtual ~FinishRunWidget();

    virtual void closeEvent(QCloseEvent *event) override;

    FinishRun finishRun() const { return m_finishRun; }
    void setFinishRun(FinishRun value);

protected slots:
    void onPushButtonOkClicked(bool checked = false);
    void onComboBoxActionsSelectedChanged(int index);

signals:
    void initialized();
    void closed();

private:
    FinishRun m_finishRun;
    QList<QCheckBox*> m_relays;
};

#endif // FINISHRUNWIDGET_H