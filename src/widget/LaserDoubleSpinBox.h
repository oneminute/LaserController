#ifndef LASERDOUBLESPINBOX_H
#define LASERDOUBLESPINBOX_H

#include <QDoubleSpinBox>
#include <QWidget>
#include <QKeyEvent>

class LaserDoubleSpinBox:public QDoubleSpinBox
{
	Q_OBJECT
public:
	explicit LaserDoubleSpinBox(QWidget *parent = nullptr);
	~LaserDoubleSpinBox();
protected:

	virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
	virtual void focusOutEvent(QFocusEvent *event);
signals:
	void enterOrLostFocus();
private:
    bool m_isPressEnterKey;
    qreal m_lastValue;
    bool m_isValueChanged;
};
#endif // LASERDOUBLESPINBOX_H

