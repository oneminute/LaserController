#ifndef LASERTOOLBUTTON_H
#define LASERTOOLBUTTON_H

#include <QToolButton> 
#include <QWidget>
#include <QMouseEvent>
class LaserToolButton : public QToolButton {
    Q_OBJECT
public:
    explicit LaserToolButton(QWidget *parent = nullptr);
    ~LaserToolButton();

    void showMenuWidget();
protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
signals:
    void showMenu();
    friend class LaserApplication;
};
#endif LASERTOOLBUTTON_H