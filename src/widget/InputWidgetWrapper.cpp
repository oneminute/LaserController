#include "InputWidgetWrapper.h"

#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTimeEdit>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QDial>
#include "widget/EditSlider.h"
#include <QLabel>

InputWidgetWrapper::InputWidgetWrapper(QWidget* widget, Config::ConfigItem* configItem, QLabel* labelName, QLabel* labelDesc)
    : QObject(widget)
    , m_labelName(labelName)
    , m_labelDesc(labelDesc)
    , m_configItem(configItem)
    , m_type(WT_Unknown)
{
    QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget);
    QTextEdit* textEdit = qobject_cast<QTextEdit*>(widget);
    QPlainTextEdit* plainTextEdit = qobject_cast<QPlainTextEdit*>(widget);
    QSpinBox* spinBox = qobject_cast<QSpinBox*>(widget);
    QDoubleSpinBox* doubleSpinBox = qobject_cast<QDoubleSpinBox*>(widget);
    QTimeEdit* timeEdit = qobject_cast<QTimeEdit*>(widget);
    QDateEdit* dateEdit = qobject_cast<QDateEdit*>(widget);
    QDateTimeEdit* dateTimeEdit = qobject_cast<QDateTimeEdit*>(widget);
    QDial* dial = qobject_cast<QDial*>(widget);
    EditSlider* editSlider = qobject_cast<EditSlider*>(widget);

    if (comboBox != nullptr)
    {
        m_type = WT_ComboBox;
        m_value = comboBox->currentText();
        connect(comboBox, &QComboBox::currentTextChanged, this, &InputWidgetWrapper::onTextChanged);
    }
    else if (lineEdit != nullptr)
    {
        m_type = WT_LineEdit;
        m_value = lineEdit->text();
        connect(lineEdit, &QLineEdit::textChanged, this, &InputWidgetWrapper::onTextChanged);
    }
    else if (textEdit != nullptr)
    {
        m_type = WT_TextEdit;
        m_value = textEdit->toPlainText();
        connect(textEdit, &QTextEdit::textChanged, this, &InputWidgetWrapper::onTextEditTextChanged);
    }
    else if (plainTextEdit != nullptr)
    {
        m_type = WT_PlainTextEdit;
        m_value = plainTextEdit->toPlainText();
        connect(plainTextEdit, &QPlainTextEdit::textChanged, this, &InputWidgetWrapper::onPlainTextEditTextChanged);
    }
    else if (spinBox != nullptr)
    {
        m_type = WT_SpinBox;
        m_value = spinBox->value();
        connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, QOverload<int>::of(&InputWidgetWrapper::onValueChanged));
    }
    else if (doubleSpinBox != nullptr)
    {
        m_type = WT_DoubleSpinBox;
        m_value = doubleSpinBox->value();
        connect(doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, QOverload<double>::of(&InputWidgetWrapper::onValueChanged));
    }
    else if (timeEdit != nullptr)
    {
        m_type = WT_TimeEdit;
        m_value = timeEdit->time();
        connect(timeEdit, &QTimeEdit::timeChanged, this, &InputWidgetWrapper::onTimeChanged);
    }
    else if (dateEdit != nullptr)
    {
        m_type = WT_DateEdit;
        m_value = dateEdit->date();
        connect(dateEdit, &QDateEdit::dateChanged, this, &InputWidgetWrapper::onDateChanged);
    }
    else if (dateTimeEdit != nullptr)
    {
        m_type = WT_DateTimeEdit;
        m_value = dateTimeEdit->dateTime();
        connect(dateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, &InputWidgetWrapper::onDateTimeChanged);
    }
    else if (dial != nullptr)
    {
        m_type = WT_Dial;
        m_value = dial->value();
        connect(dial, &QDial::valueChanged, this, QOverload<int>::of(&InputWidgetWrapper::onValueChanged));
    }
    else if (editSlider != nullptr)
    {
        m_type = WT_EditSlider;
        m_value = editSlider->value();
        connect(editSlider, &EditSlider::valueChanged, this, QOverload<int>::of(&InputWidgetWrapper::onValueChanged));
    }
    restore();
}

InputWidgetWrapper::~InputWidgetWrapper()
{
}

void InputWidgetWrapper::updateConfigItem()
{
    m_configItem->value = m_value;
    m_modified = false;
}

void InputWidgetWrapper::restore()
{
    QWidget* widget = qobject_cast<QWidget*>(parent());

    switch (m_type)
    {
    case WT_ComboBox:
    {
        QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
        comboBox->setCurrentText(m_configItem->value.toString());
    }
        break;
    case WT_LineEdit:
    {
        QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget);
        lineEdit->setText(m_configItem->value.toString());
    }
        break;
    case WT_TextEdit:
    {
        QTextEdit* textEdit = qobject_cast<QTextEdit*>(widget);
        textEdit->setText(m_configItem->value.toString());
    }
        break;
    case WT_PlainTextEdit:
    {
        QPlainTextEdit* plainTextEdit = qobject_cast<QPlainTextEdit*>(widget);
        plainTextEdit->setPlainText(m_configItem->value.toString());
    }
        break;
    case WT_SpinBox:
    {
        QSpinBox* spinBox = qobject_cast<QSpinBox*>(widget);
        spinBox->setValue(m_configItem->value.toInt());
    }
        break;
    case WT_DoubleSpinBox:
    {
        QDoubleSpinBox* doubleSpinBox = qobject_cast<QDoubleSpinBox*>(widget);
        doubleSpinBox->setValue(m_configItem->value.toDouble());
    }
        break;
    case WT_TimeEdit:
    {
        QTimeEdit* timeEdit = qobject_cast<QTimeEdit*>(widget);
        timeEdit->setTime(m_configItem->value.toTime());
    }
        break;
    case WT_DateEdit:
    {
        QDateEdit* dateEdit = qobject_cast<QDateEdit*>(widget);
        dateEdit->setDate(m_configItem->value.toDate());
    }
        break;
    case WT_DateTimeEdit:
    {
        QDateTimeEdit* dateTimeEdit = qobject_cast<QDateTimeEdit*>(widget);
        dateTimeEdit->setDateTime(m_configItem->value.toDateTime());
    }
        break;
    case WT_Dial:
    {
        QDial* dial = qobject_cast<QDial*>(widget);
        dial->setValue(m_configItem->value.toInt());
    }
        break;
    case WT_EditSlider:
    {
        EditSlider* editSlider = qobject_cast<EditSlider*>(widget);
        editSlider->setValue(m_configItem->value.toInt());
    }
        break;
    }
}

void InputWidgetWrapper::restoreDefault()
{
    QWidget* widget = qobject_cast<QWidget*>(parent());

    switch (m_type)
    {
    case WT_ComboBox:
    {
        QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
        comboBox->setCurrentText(m_configItem->defaultValue.toString());
    }
        break;
    case WT_LineEdit:
    {
        QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget);
        lineEdit->setText(m_configItem->defaultValue.toString());
    }
        break;
    case WT_TextEdit:
    {
        QTextEdit* textEdit = qobject_cast<QTextEdit*>(widget);
        textEdit->setText(m_configItem->defaultValue.toString());
    }
        break;
    case WT_PlainTextEdit:
    {
        QPlainTextEdit* plainTextEdit = qobject_cast<QPlainTextEdit*>(widget);
        plainTextEdit->setPlainText(m_configItem->defaultValue.toString());
    }
        break;
    case WT_SpinBox:
    {
        QSpinBox* spinBox = qobject_cast<QSpinBox*>(widget);
        spinBox->setValue(m_configItem->defaultValue.toInt());
    }
        break;
    case WT_DoubleSpinBox:
    {
        QDoubleSpinBox* doubleSpinBox = qobject_cast<QDoubleSpinBox*>(widget);
        doubleSpinBox->setValue(m_configItem->defaultValue.toDouble());
    }
        break;
    case WT_TimeEdit:
    {
        QTimeEdit* timeEdit = qobject_cast<QTimeEdit*>(widget);
        timeEdit->setTime(m_configItem->defaultValue.toTime());
    }
        break;
    case WT_DateEdit:
    {
        QDateEdit* dateEdit = qobject_cast<QDateEdit*>(widget);
        dateEdit->setDate(m_configItem->defaultValue.toDate());
    }
        break;
    case WT_DateTimeEdit:
    {
        QDateTimeEdit* dateTimeEdit = qobject_cast<QDateTimeEdit*>(widget);
        dateTimeEdit->setDateTime(m_configItem->defaultValue.toDateTime());
    }
        break;
    case WT_Dial:
    {
        QDial* dial = qobject_cast<QDial*>(widget);
        dial->setValue(m_configItem->defaultValue.toInt());
    }
        break;
    case WT_EditSlider:
    {
        EditSlider* editSlider = qobject_cast<EditSlider*>(widget);
        editSlider->setValue(m_configItem->defaultValue.toInt());
    }
        break;
    }
}

bool InputWidgetWrapper::isModified()
{
    return m_modified;
}

void InputWidgetWrapper::onTextChanged(const QString & text)
{
    setValue(text);
}

void InputWidgetWrapper::onTextEditTextChanged()
{
    QTextEdit* textEdit = qobject_cast<QTextEdit*>(sender());
    QString text = textEdit->toPlainText();
    setValue(text);
}

void InputWidgetWrapper::onPlainTextEditTextChanged()
{
    QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(sender());
    QString text = textEdit->toPlainText();
    setValue(text);
}

void InputWidgetWrapper::onValueChanged(int value)
{
    setValue(value);
}

void InputWidgetWrapper::onValueChanged(double value)
{
    setValue(value);
}

void InputWidgetWrapper::onTimeChanged(const QTime & time)
{
    setValue(time);
}

void InputWidgetWrapper::onDateChanged(const QDate & date)
{
    setValue(date);
}

void InputWidgetWrapper::onDateTimeChanged(const QDateTime & dateTime)
{
    setValue(dateTime);
}

void InputWidgetWrapper::setValue(const QVariant & value)
{
	if (!m_labelName)
		return;

    QPalette::ColorRole role = m_labelName->foregroundRole();
    QPalette palette = m_labelName->palette();

    if (m_configItem->value == value)
    {
        m_modified = false;
        palette.setColor(role, Qt::black);
    }
    else
    {
        m_modified = true;
        palette.setColor(role, Qt::red);
    }

    m_labelName->setPalette(palette);
    m_value = value;
    emit valueChanged(value);
}


