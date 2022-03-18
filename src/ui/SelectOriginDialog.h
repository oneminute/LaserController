#ifndef SELECTORIGINDIALOG_H
#define SELECTORIGINDIALOG_H

#include <QDialog>

class RadioButtonGroup;

class SelectOriginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SelectOriginDialog(QWidget* parent = nullptr);
    ~SelectOriginDialog();

    int origin() const;

private:
    RadioButtonGroup* m_buttonGroupOrigin;
    Q_DISABLE_COPY(SelectOriginDialog)
};

#endif // SELECTORIGINDIALOG_H