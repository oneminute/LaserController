#ifndef RADIOBUTTONGROUP_H
#define RADIOBUTTONGROUP_H

#include <QObject>
#include <QRadioButton>
#include <QList>
#include <QGridLayout>

class RadioButtonGroupPrivate;
class RadioButtonGroup : public QWidget
{
    Q_OBJECT
public:
    explicit RadioButtonGroup(int rows = 3, int cols = 3, QWidget* parent = nullptr);
    ~RadioButtonGroup();

    int rows() const;
    int cols() const;
    int count() const;

    void setRows(int rows);
    void setCols(int cols);
    void setRowsCols(int rows, int cols);

    int value() const;
    int value(int row, int col) const;
    int value(int index) const;
    QList<int> values() const;

    void setValue(int value);
    void setValue(int row, int col, int value);
    void setValue(int index, int value);
    void setValues(const QList<int>& values);

public slots:
    void updateLayout();

protected:
    void clear();

signals:
    void layoutUpdated();
    void valueChanged(int value);

private:
    QScopedPointer<RadioButtonGroupPrivate> d_ptr;
    Q_DECLARE_PRIVATE(RadioButtonGroup)
    Q_DISABLE_COPY(RadioButtonGroup)
};

#endif // RADIOBUTTONGROUP_H
