#ifndef OVERSTEPMESSAGEBOXWARN_H
#define OVERSTEPMESSAGEBOXWARN_H
#include <QMessageBox>
class OverstepMessageBoxWarn :  public QMessageBox {
    Q_OBJECT
public:
    OverstepMessageBoxWarn(QWidget *parent = nullptr);
    ~OverstepMessageBoxWarn();
protected:

    virtual void keyPressEvent(QKeyEvent *e) override;

    virtual void keyReleaseEvent(QKeyEvent* event) override;
};
#endif // OVERSTEPMESSAGEBOXWARN_H