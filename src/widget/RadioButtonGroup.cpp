#include "RadioButtonGroup.h"

#include "common/common.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QtMath>

class RadioButtonGroupPrivate
{
    Q_DECLARE_PUBLIC(RadioButtonGroup)
public:
    RadioButtonGroupPrivate(RadioButtonGroup* ptr)
        : q_ptr(ptr)
        , rows(4)
        , cols(4)
        , gridLayout(nullptr)
        , currentIndex(0)
    {}

    ~RadioButtonGroupPrivate()
    {}

    RadioButtonGroup* q_ptr;

    int rows;
    int cols;
    QList<QRadioButton*> buttons;
    QGridLayout* gridLayout;
    QHBoxLayout* layout;
    QList<int> values;
    int currentIndex;
};

RadioButtonGroup::RadioButtonGroup(int rows, int cols, QWidget* parent)
    : QWidget(parent)
    , d_ptr(new RadioButtonGroupPrivate(this))
{
    Q_D(RadioButtonGroup);
    d->rows = rows;
    d->cols = cols;
    d->layout = new QHBoxLayout;
    d->gridLayout = new QGridLayout;
    d->layout->addLayout(d->gridLayout);
    d->layout->addStretch(1);
    setLayout(d->layout);
    updateLayout();
}

RadioButtonGroup::~RadioButtonGroup()
{
}

int RadioButtonGroup::rows() const
{
    Q_D(const RadioButtonGroup);
    return d->rows;
}

int RadioButtonGroup::cols() const
{
    Q_D(const RadioButtonGroup);
    return d->cols;
}

int RadioButtonGroup::count() const
{
    Q_D(const RadioButtonGroup);
    return d->rows * d->cols;
}

void RadioButtonGroup::setRows(int rows)
{
    Q_D(RadioButtonGroup);
    int col = d->currentIndex % d->cols;
    int row = d->currentIndex / d->rows;
    row = qBound(0, qRound(row * 1.0 * rows / d->rows), rows - 1);
    d->currentIndex = row * d->cols + col;
    d->rows = rows;
    clear();
    updateLayout();
}

void RadioButtonGroup::setCols(int cols)
{
    Q_D(RadioButtonGroup);
    int col = d->currentIndex % d->cols;
    int row = d->currentIndex / d->rows;
    col = qBound(0, qRound(col * 1.0 * cols / d->cols), cols - 1);
    d->currentIndex = row * d->cols + col;
    d->cols = cols;
    clear();
    updateLayout();
}

void RadioButtonGroup::setRowsCols(int rows, int cols)
{
    Q_D(RadioButtonGroup);
    int col = d->currentIndex % d->cols;
    int row = d->currentIndex / d->rows;
    row = qBound(0, qRound(row * 1.0 * rows / d->rows), rows - 1);
    col = qBound(0, qRound(col * 1.0 * cols / d->cols), cols - 1);
    d->rows = rows;
    d->cols = cols;
    d->currentIndex = row * d->cols + col;
    clear();
    updateLayout();
}

int RadioButtonGroup::value() const
{
    Q_D(const RadioButtonGroup);
    return d->values.at(d->currentIndex);
}

int RadioButtonGroup::value(int row, int col) const
{
    Q_D(const RadioButtonGroup);
    int index = row * d->cols + col;
    return d->values.at(index);
}

int RadioButtonGroup::value(int index) const
{
    Q_D(const RadioButtonGroup);
    index = qBound(0, index, count() - 1);
    return d->values.at(index);
}

QList<int> RadioButtonGroup::values() const
{
    Q_D(const RadioButtonGroup);
    return d->values;
}

void RadioButtonGroup::setValue(int value)
{
    Q_D(RadioButtonGroup);
    int index = d->values.indexOf(value);
    if (index >= 0)
    {
        bool changed = d->currentIndex != index;
        d->currentIndex = index;
        d->buttons.at(index)->setChecked(true);
        if (changed)
            emit valueChanged(value);
    }
}

void RadioButtonGroup::setValue(int row, int col, int value)
{
    Q_D(RadioButtonGroup);
    int index = row * d->cols + col;
    index = qBound(0, index, count() - 1);
    bool changed = d->values[index] != value;
    d->values[index] = value;
    if (changed)
        emit valueChanged(value);
}

void RadioButtonGroup::setValue(int index, int value)
{
    Q_D(RadioButtonGroup);
    index = qBound(0, index, count() - 1);
    bool changed = d->values[index] != value;
    d->values[index] = value;
    if (changed)
        emit valueChanged(value);
}

void RadioButtonGroup::setValues(const QList<int>& values)
{
    Q_D(RadioButtonGroup);
    if (values.length() == d->values.length())
    {
        bool changed = values.at(d->currentIndex) != d->values.at(d->currentIndex);
        d->values = values;
        if (changed)
            emit valueChanged(d->values.at(d->currentIndex));
    }
}

void RadioButtonGroup::updateLayout()
{
    Q_D(RadioButtonGroup);
    int index = 0;
    for (int r = 0; r < d->rows; r++)
    {
        for (int c = 0; c < d->cols; c++)
        {
            QRadioButton* button = new QRadioButton(this);
            button->setText("");
            QString hint = QString("%1").arg(index);
            button->setToolTip(hint);
            d->values.append(index);
            d->buttons.append(button);
            connect(button, &QRadioButton::toggled,
                [=](bool checked) {
                    d->currentIndex = index;
                }
            );
            if (index == d->currentIndex)
                button->setChecked(true);
            d->gridLayout->addWidget(button, r, c);
            index++;
        }
    }
    emit valueChanged(d->values.at(d->currentIndex));
}

void RadioButtonGroup::clear()
{
    Q_D(RadioButtonGroup);
    for (QRadioButton* button : d->buttons)
    {
        d->gridLayout->removeWidget(button);
        delete button;
    }
    d->buttons.clear();
    d->values.clear();
}
