#ifndef LASERMENU_H
#define LASERMENU_H
#include<QMenu>
class LaserMenu : public QMenu {
    Q_OBJECT
public :
    explicit LaserMenu(QWidget *parent = nullptr);
    ~LaserMenu();

    virtual void
        keyPressEvent(QKeyEvent *e) override;

};
#endif