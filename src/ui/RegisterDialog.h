#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H
#include <QMainWindow>

class RegisterDialog : public QMainWindow {
    Q_OBJECT
public:
    RegisterDialog(QWidget* parent);
    ~RegisterDialog();
};
#endif