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
signals:
	void enterOrLostFocus();


};
#endif // LASERDOUBLESPINBOX_H

