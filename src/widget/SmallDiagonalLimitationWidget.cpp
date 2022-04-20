#include "SmallDiagonalLimitationWidget.h"
#include "common/common.h"
#include <QDoubleSpinBox>
#include <QLabel>
#include <QToolButton>
#include "LaserApplication.h"

SmallDiagonalLimitationWidget::SmallDiagonalLimitationWidget(QWidget* parent)
    : QWidget(parent)
{
    init();
}

SmallDiagonalLimitationWidget::SmallDiagonalLimitationWidget(ConfigItem* item, QWidget* parent)
    : QWidget(parent)
    , m_layout(nullptr)
    , m_item(item)
{
    init();
}

void SmallDiagonalLimitationWidget::init()
{
    m_limitation = m_item->value().value<SmallDiagonalLimitation*>();

    m_layout = new QVBoxLayout;
    m_layout->setMargin(0);
    m_layout->setSpacing(2);

    m_gridLayout = new QGridLayout;
    updateLimitations();
    
    QToolButton* addButton = new QToolButton;
    addButton->setText(tr("Add limitation item"));
    connect(addButton, &QToolButton::clicked, this, &SmallDiagonalLimitationWidget::onAddButtonClicked);

    m_layout->addLayout(m_gridLayout);
    m_layout->addWidget(addButton);

    setLayout(m_layout);
}

void SmallDiagonalLimitationWidget::updateLimitations()
{
    int rows = 0;
    for (LimitationMap::Iterator i = m_limitation->begin(); i != m_limitation->end(); i++)
    {
        int row = rows;
        addRow(row, &i.value());

        rows++;
    }
}

void SmallDiagonalLimitationWidget::addRow(int row, SmallDiagonalLimitationItem* item)
{
    QLabel* labelDiagonal = new QLabel(ltr("Size"));
    QDoubleSpinBox* dsbDiagonal = new QDoubleSpinBox;
    dsbDiagonal->setMinimum(0);
    dsbDiagonal->setMaximum(1000000);
    dsbDiagonal->setSingleStep(0.1);
    dsbDiagonal->setValue(item->diagonal);
    connect(dsbDiagonal, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [=](double value) {
            item->diagonal = value;
            emitValueChanged();
        });
    m_gridLayout->addWidget(labelDiagonal, row, 0);
    m_gridLayout->setAlignment(labelDiagonal, Qt::AlignmentFlag::AlignRight);
    m_gridLayout->addWidget(dsbDiagonal, row, 1);

    QLabel* labelSpeed = new QLabel(ltr("Speed"));
    QDoubleSpinBox* dsbSpeed = new QDoubleSpinBox;
    dsbSpeed->setMinimum(0);
    dsbSpeed->setMaximum(1000);
    dsbSpeed->setDecimals(3);
    dsbSpeed->setValue(item->speed);
    connect(dsbSpeed, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [=](double value) {
            item->speed = value;
            emitValueChanged();
        });
    m_gridLayout->addWidget(labelSpeed, row, 2);
    m_gridLayout->setAlignment(labelSpeed, Qt::AlignmentFlag::AlignRight);
    m_gridLayout->addWidget(dsbSpeed, row, 3);

    QToolButton* removeButton = new QToolButton;
    removeButton->setText("-");
    connect(removeButton, &QToolButton::clicked,
        [=](bool checked) {
            int index = m_gridLayout->indexOf(removeButton);
            int iRow, iCol, span;
            m_gridLayout->getItemPosition(index, &iRow, &iCol, &span, &span);
            qLogD << "removing row: " << iRow;
            removeRow(iRow);
            m_limitation->remove(item->diagonal);
            emitValueChanged();
        });
    m_gridLayout->addWidget(removeButton, row, 4);

    m_gridLayout->setColumnStretch(0, 1);
    m_gridLayout->setColumnStretch(1, 2);
    m_gridLayout->setColumnStretch(2, 1);
    m_gridLayout->setColumnStretch(3, 2);
    m_gridLayout->setColumnStretch(4, 0);
    m_gridLayout->setColumnStretch(5, 10);
}

void SmallDiagonalLimitationWidget::removeRow(int row)
{
    for (int i = 0; i < m_gridLayout->columnCount(); i++)
    {
        QLayoutItem* item = m_gridLayout->itemAtPosition(row, i);
        if (!item)
            continue;
        m_gridLayout->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }

    for (int i = row + 1; i < m_limitation->count(); i++)
    {
        for (int j = 0; j < m_gridLayout->columnCount(); j++)
        {
            QLayoutItem* item = m_gridLayout->itemAtPosition(i, j);
            if (!item)
                continue;
            m_gridLayout->removeItem(item);
            m_gridLayout->addItem(item, i - 1, j);
        }
    }
}

void SmallDiagonalLimitationWidget::emitValueChanged()
{
    emit valueChanged(m_limitation);
    emit m_item->modifiedChanged(true);
}

void SmallDiagonalLimitationWidget::clearWidgets()
{
    while (!m_gridLayout->isEmpty())
    {
        QLayoutItem* item = m_gridLayout->takeAt(0);
        if (item->widget())
            delete item->widget();
        else if (item->layout())
            delete item->layout();
        delete item;
    }
}

void SmallDiagonalLimitationWidget::onAddButtonClicked(bool checked)
{
    int row = m_limitation->count();
    SmallDiagonalLimitationItem& item = m_limitation->createNewItem();
    addRow(row, &item);
    emitValueChanged();
}
