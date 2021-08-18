#include "SmallDiagonalLimitationWidget.h"
#include "common/common.h"
#include <QDoubleSpinBox>
#include <QLabel>
#include <QToolButton>

SmallDiagonalLimitationWidget::SmallDiagonalLimitationWidget(QWidget* parent)
    : QWidget(parent)
{
    init();
}

SmallDiagonalLimitationWidget::SmallDiagonalLimitationWidget(const SmallDiagonalLimitation& limitation, QWidget* parent)
    : QWidget(parent)
    , m_limitation(limitation)
{
    init();
}

void SmallDiagonalLimitationWidget::init()
{
    m_layout = new QGridLayout;
    int rows = 0;
    for (SmallDiagonalLimitationItem& item : m_limitation.items())
    {
        int row = rows;
        addRow(row, item);

        rows++;
    }
    QToolButton* addButton = new QToolButton;
    m_layout->addWidget(addButton, rows, 0, 1, 7);

    setLayout(m_layout);
}

void SmallDiagonalLimitationWidget::addRow(int row, const SmallDiagonalLimitationItem& item)
{
    QLabel* labelDiagonal = new QLabel(tr("Diagonal"));
    QDoubleSpinBox* dsbDiagonal = new QDoubleSpinBox;
    dsbDiagonal->setMinimum(0);
    dsbDiagonal->setMaximum(1000000);
    dsbDiagonal->setValue(item.diagonal);
    connect(dsbDiagonal, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [this, row](double value) {
            qLogD << "row: " << row;
            m_limitation.items()[row].diagonal = value;
        });
    m_layout->addWidget(labelDiagonal, row, 0);
    m_layout->addWidget(dsbDiagonal, row, 1);

    QLabel* labelPower = new QLabel(tr("Power"));
    QDoubleSpinBox* dsbPower = new QDoubleSpinBox;
    dsbPower->setMinimum(0);
    dsbPower->setMaximum(100);
    dsbPower->setDecimals(1);
    dsbPower->setValue(item.laserPower);
    m_layout->addWidget(labelPower, row, 2);
    m_layout->addWidget(dsbPower, row, 3);
    connect(dsbPower, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [this, row](double value) {
            qLogD << "row: " << row;
            m_limitation.items()[row].laserPower = value;
        });

    QLabel* labelSpeed = new QLabel(tr("Speed"));
    QDoubleSpinBox* dsbSpeed = new QDoubleSpinBox;
    dsbSpeed->setMinimum(0);
    dsbSpeed->setMaximum(1000);
    dsbSpeed->setDecimals(5);
    dsbSpeed->setValue(item.speed);
    m_layout->addWidget(labelSpeed, row, 4);
    m_layout->addWidget(dsbSpeed, row, 5);
    connect(dsbSpeed, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [this, row](double value) {
            qLogD << "row: " << row;
            m_limitation.items()[row].speed = value;
        });

    QToolButton* removeButton = new QToolButton;
    removeButton->setText("-");
    m_layout->addWidget(removeButton, row, 6);
}

void SmallDiagonalLimitationWidget::onAddButtonClicked(bool checked)
{
    
}
