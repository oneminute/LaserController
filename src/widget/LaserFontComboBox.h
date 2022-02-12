#ifndef LASERFONTCOMBOBOX_H
#define LASERFONTCOMBOBOX_H
#include <QFontComboBox>
#include <QWidget>
class LaserFontComboBox : public QFontComboBox {
    Q_OBJECT
    public:
        LaserFontComboBox(QWidget *parent = nullptr);
        virtual ~LaserFontComboBox();
        virtual void showPopup() override;
        virtual void hidePopup() override;
        /*virtual void focusOutEvent(QFocusEvent *e) override;
        virtual void leaveEvent(QEvent *event) override;
        virtual void mouseReleaseEvent(QMouseEvent *e) override;


        void setIsChangedItem(bool bl);
        bool isChangedItem();*/

    signals:
        //void outFocus();
        void hidePopupSignal();
    
private :
    bool m_isShowPopup;
};
#endif