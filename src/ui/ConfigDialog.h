#ifndef CONFIG_DIALOG_H
#define CONFIG_DIALOG_H

#include <QDialog>
#include <QScopedPointer>

namespace Ui
{
    class ConfigDialog;
}

class ConfigDialog: public QDialog
{
    Q_OBJECT
public:
    ConfigDialog(QWidget* parent = nullptr);
    virtual ~ConfigDialog();

private:
};

#endif // CONFIG_DIALOG