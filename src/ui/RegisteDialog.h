#ifndef REGISTEDIALOG_H 
#define REGISTEDIALOG_H 

#include <QDialog> 

namespace Ui
{
    class RegisteDialog;
}
 
class RegisteDialog : public QDialog 
{ 
    Q_OBJECT 
public: 
    RegisteDialog(QWidget* parent = nullptr); 
    ~RegisteDialog();

protected slots:
    void onButtonCopyDongleIdClicked(bool checked = false);
    void onButtonCopyRegisteIdClicked(bool checked = false);
    void onButtonRegiste(bool checked = false);

private:
    QScopedPointer<Ui::RegisteDialog> m_ui;
}; 
#endif