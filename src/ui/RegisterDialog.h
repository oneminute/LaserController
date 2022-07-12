#ifndef REGISTERDIALOG_H 
#define REGISTERDIALOG_H 

#include <QDialog> 

namespace Ui
{
    class RegisterDialog;
}
 
class RegisterDialog : public QDialog 
{ 
    Q_OBJECT 
public: 
    RegisterDialog(QWidget* parent = nullptr); 
    ~RegisterDialog();

protected slots:
    void onButtonCopyDongleIdClicked(bool checked = false);
    void onButtonCopyRegisteIdClicked(bool checked = false);
    void onButtonRegiste(bool checked = false);
    void onButtonStatusClicked(bool checked = false);
    void onRegistrationChanged(bool registered);

private:
    QScopedPointer<Ui::RegisterDialog> m_ui;
}; 
#endif