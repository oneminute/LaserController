#pragma once

#include <QDialog>

namespace Ui
{
    class ImportDialog;
}

class ImportDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ImportDialog(QWidget* parent = nullptr);
    ~ImportDialog();

    qreal factor() const;

    bool disableViewbox() const;

protected slots:
    void currentIndexChanged(int index);

private:
    QScopedPointer<Ui::ImportDialog> m_ui;
};