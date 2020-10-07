#include "FinishRunWidget.h"
#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>

FinishRunWidget::FinishRunWidget(QWidget* parent)
    : QWidget(parent)
{
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::Direction::TopToBottom);
    setLayout(layout);

    QBoxLayout* relaysLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    layout->addLayout(relaysLayout);

    QBoxLayout* actionLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    layout->addLayout(actionLayout);

    QComboBox* comboBoxActions = new QComboBox(this);
    actionLayout->addWidget(comboBoxActions);
    QPushButton* buttonOk = new QPushButton(this);
    buttonOk->setText(tr("Ok"));
    actionLayout->addWidget(buttonOk);
    actionLayout->addStretch(0);

    comboBoxActions->addItem(tr("None"));
    comboBoxActions->addItem(tr("Release"));
    comboBoxActions->addItem(tr("Origin"));
    comboBoxActions->addItem(tr("Machining1"));
    comboBoxActions->addItem(tr("Machining2"));
    comboBoxActions->addItem(tr("Machining3"));

    for (int i = 0; i < 8; i++)
    {
        QString relayName = QString("R%1").arg(i + 1);
        QCheckBox* checkBoxRelay = new QCheckBox(this);
        m_relays.append(checkBoxRelay);
        checkBoxRelay->setText(relayName);
        connect(checkBoxRelay, &QCheckBox::toggled, [=](bool checked = true)
            {
                m_finishRun.setRelay(i, checked);
                qDebug() << m_finishRun.code << m_finishRun.action << m_finishRun.relays;
            }
        );
        relaysLayout->addWidget(checkBoxRelay);
    }

    connect(comboBoxActions, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &FinishRunWidget::onComboBoxActionsSelectedChanged);
    connect(buttonOk, &QPushButton::clicked, this, &FinishRunWidget::onPushButtonOkClicked);

    emit initialized();
}

FinishRunWidget::~FinishRunWidget()
{
}

void FinishRunWidget::closeEvent(QCloseEvent * event)
{
    qDebug() << "FinishRunWidget::closeEvent";
    emit closed();
}

void FinishRunWidget::setFinishRun(FinishRun value) { m_finishRun = value; }

void FinishRunWidget::onPushButtonOkClicked(bool checked)
{
    this->close();
}

void FinishRunWidget::onComboBoxActionsSelectedChanged(int index)
{
    m_finishRun.setAction(index);
    for (int i = 0; i < 8; i++)
    {
        if (m_finishRun.isEnabled(i))
        {
            m_relays[i]->blockSignals(true);
            m_relays[i]->setChecked(true);
            m_relays[i]->blockSignals(false);
        }
    }
    qDebug() << m_finishRun.code << m_finishRun.action << m_finishRun.relays;
}

