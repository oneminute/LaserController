#include "InputWidgetWrapper.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QDial>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QTextEdit>
#include <QTimeEdit>
#include "util/TypeUtils.h"
#include "widget/EditSlider.h"
#include "widget/FloatEditSlider.h"

InputWidgetWrapper::InputWidgetWrapper(QWidget* widget, ConfigItem* configItem)
    : QObject(widget)
    , m_labelName(nullptr)
    , m_labelDesc(nullptr)
    , m_configItem(configItem)
    , m_type(IWT_Unknown)
{
    QCheckBox* checkBox = qobject_cast<QCheckBox*>(widget);
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
    FloatEditSlider* floatEditSlider = qobject_cast<FloatEditSlider*>(widget);

    if (checkBox != nullptr)
    {
        m_type = IWT_CheckBox;
        connect(checkBox, &QCheckBox::stateChanged, this, &InputWidgetWrapper::onCheckBoxStateChanged);
    }
    else if (comboBox != nullptr)
    {
        m_type = IWT_ComboBox;
        connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InputWidgetWrapper::onComboBoxIndexChanged);
    }
    else if (lineEdit != nullptr)
    {
        m_type = IWT_LineEdit;
        connect(lineEdit, &QLineEdit::editingFinished, this, &InputWidgetWrapper::onEditingFinished);
    }
    else if (textEdit != nullptr)
    {
        m_type = IWT_TextEdit;
        connect(textEdit, &QTextEdit::textChanged, this, &InputWidgetWrapper::onTextEditTextChanged);
    }
    else if (plainTextEdit != nullptr)
    {
        m_type = IWT_PlainTextEdit;
        connect(plainTextEdit, &QPlainTextEdit::textChanged, this, &InputWidgetWrapper::onPlainTextEditTextChanged);
    }
    else if (spinBox != nullptr)
    {
        m_type = IWT_SpinBox;
        connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, QOverload<int>::of(&InputWidgetWrapper::onValueChanged));
    }
    else if (doubleSpinBox != nullptr)
    {
        m_type = IWT_DoubleSpinBox;
        connect(doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, QOverload<double>::of(&InputWidgetWrapper::onValueChanged));
    }
    else if (timeEdit != nullptr)
    {
        m_type = IWT_TimeEdit;
        connect(timeEdit, &QTimeEdit::timeChanged, this, &InputWidgetWrapper::onTimeChanged);
    }
    else if (dateEdit != nullptr)
    {
        m_type = IWT_DateEdit;
        connect(dateEdit, &QDateEdit::dateChanged, this, &InputWidgetWrapper::onDateChanged);
    }
    else if (dateTimeEdit != nullptr)
    {
        m_type = IWT_DateTimeEdit;
        connect(dateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, &InputWidgetWrapper::onDateTimeChanged);
    }
    else if (dial != nullptr)
    {
        m_type = IWT_Dial;
        connect(dial, &QDial::valueChanged, this, QOverload<int>::of(&InputWidgetWrapper::onValueChanged));
    }
    else if (editSlider != nullptr)
    {
        m_type = IWT_EditSlider;
        connect(editSlider, &EditSlider::valueChanged, this, QOverload<int>::of(&InputWidgetWrapper::onValueChanged));
    }
    else if (floatEditSlider != nullptr)
    {
        m_type = IWT_FloatEditSlider;
        connect(floatEditSlider, &FloatEditSlider::valueChanged, this, QOverload<qreal>::of(&InputWidgetWrapper::onValueChanged));
    }
    updateValue(m_configItem->value());
    connect(this, &InputWidgetWrapper::valueChanged, m_configItem, &ConfigItem::fromWidget);
    connect(m_configItem, &ConfigItem::modifiedChanged, this, &InputWidgetWrapper::onConfigItemModifiedChanged);
    connect(m_configItem, &ConfigItem::widgetValueChanged, this, &InputWidgetWrapper::onConfigItemValueChanged);
}

InputWidgetWrapper::~InputWidgetWrapper()
{
}

void InputWidgetWrapper::setNameLabel(QLabel* label)
{
    m_labelName = label;
}

void InputWidgetWrapper::setDescriptionLabel(QLabel* label)
{
    m_labelDesc = label;
}

QWidget* InputWidgetWrapper::widget() const
{
    return qobject_cast<QWidget*>(parent());
}

void InputWidgetWrapper::reset()
{
    m_configItem->reset();
    updateValue(m_configItem->value());
}

void InputWidgetWrapper::restoreDefault()
{
    updateValue(m_configItem->defaultValue());
}

void InputWidgetWrapper::updateValue(const QVariant& newValue)
{
    QVariant value = m_configItem->doLoadDataHook(newValue);
    QWidget* widget = qobject_cast<QWidget*>(parent());
    widget->blockSignals(true);
    switch (m_type)
    {
    case IWT_CheckBox:
    {
        QCheckBox* checkBox = qobject_cast<QCheckBox*>(widget);
        checkBox->setCheckState(value.toBool() ? Qt::Checked : Qt::Unchecked);
        break;
    }
    case IWT_ComboBox:
    {
        QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
        comboBox->setCurrentText(value.toString());
        break;
    }
    case IWT_LineEdit:
    {
        QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget);
        lineEdit->setText(value.toString());
        break;
    }
    case IWT_TextEdit:
    {
        QTextEdit* textEdit = qobject_cast<QTextEdit*>(widget);
        textEdit->setText(value.toString());
        break;
    }
    case IWT_PlainTextEdit:
    {
        QPlainTextEdit* plainTextEdit = qobject_cast<QPlainTextEdit*>(widget);
        plainTextEdit->setPlainText(value.toString());
        break;
    }
    case IWT_SpinBox:
    {
        QSpinBox* spinBox = qobject_cast<QSpinBox*>(widget);
        spinBox->setValue(value.toInt());
        break;
    }
    case IWT_DoubleSpinBox:
    {
        QDoubleSpinBox* doubleSpinBox = qobject_cast<QDoubleSpinBox*>(widget);
        doubleSpinBox->setValue(value.toDouble());
        break;
    }
    case IWT_TimeEdit:
    {
        QTimeEdit* timeEdit = qobject_cast<QTimeEdit*>(widget);
        timeEdit->setTime(value.toTime());
        break;
    }
    case IWT_DateEdit:
    {
        QDateEdit* dateEdit = qobject_cast<QDateEdit*>(widget);
        dateEdit->setDate(value.toDate());
        break;
    }
    case IWT_DateTimeEdit:
    {
        QDateTimeEdit* dateTimeEdit = qobject_cast<QDateTimeEdit*>(widget);
        dateTimeEdit->setDateTime(value.toDateTime());
        break;
    }
    case IWT_Dial:
    {
        QDial* dial = qobject_cast<QDial*>(widget);
        dial->setValue(value.toInt());
        break;
    }
    case IWT_EditSlider:
    {
        EditSlider* editSlider = qobject_cast<EditSlider*>(widget);
        editSlider->setValue(value.toInt());
        break;
    }
    case IWT_FloatEditSlider:
    {
        FloatEditSlider* editSlider = qobject_cast<FloatEditSlider*>(widget);
        editSlider->setValue(value.toReal());
        break;
    }
    }
    widget->blockSignals(false);
}

void InputWidgetWrapper::changeValue(const QVariant& value)
{
    QVariant newValue = m_configItem->doSaveDataHook(value);
    emit valueChanged(newValue);
}

bool InputWidgetWrapper::isModified()
{
    return m_configItem->isModified();
}

QVariant InputWidgetWrapper::value() const
{
    return m_configItem->value();
}

QWidget* InputWidgetWrapper::createWidget(InputWidgetType widgetType, Qt::Orientation orientation)
{
    QWidget* widget = nullptr;
    switch (widgetType)
    {
    case IWT_CheckBox:
        widget = new QCheckBox;
        break;
    case IWT_ComboBox:
        widget = new QComboBox;
        break;
    case IWT_LineEdit:
        widget = new QLineEdit;
        break;
    case IWT_TextEdit:
        widget = new QTextEdit;
        break;
    case IWT_PlainTextEdit:
        widget = new QPlainTextEdit;
        break;
    case IWT_SpinBox:
        widget = new QSpinBox;
        break;
    case IWT_DoubleSpinBox:
        widget = new QDoubleSpinBox;
        break;
    case IWT_TimeEdit:
        widget = new QTimeEdit;
        break;
    case IWT_DateEdit:
        widget = new QDateEdit;
        break;
    case IWT_DateTimeEdit:
        widget = new QDateTimeEdit;
        break;
    case IWT_Dial:
        widget = new QDial;
        break;
    case IWT_EditSlider:
        widget = new EditSlider(orientation);
        break;
    case IWT_FloatEditSlider:
        widget = new FloatEditSlider(orientation);
        break;
    default:
        widget = nullptr;
        break;
    }
    return widget;
}

void InputWidgetWrapper::onTextChanged(const QString & text)
{
    changeValue(typeUtils::textToVariant(text, m_configItem->dataType()));
}

void InputWidgetWrapper::onCheckBoxStateChanged(int state)
{
    QCheckBox* checkBox = qobject_cast<QCheckBox*>(sender());
    changeValue(state == Qt::Checked);
}

void InputWidgetWrapper::onComboBoxIndexChanged(int index)
{
    QComboBox* comboBox = qobject_cast<QComboBox*>(sender());
    changeValue(comboBox->currentData());
}

void InputWidgetWrapper::onTextEditTextChanged()
{
    QTextEdit* textEdit = qobject_cast<QTextEdit*>(sender());
    QString text = textEdit->toPlainText();
    changeValue(typeUtils::textToVariant(text, m_configItem->dataType()));
}

void InputWidgetWrapper::onPlainTextEditTextChanged()
{
    QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(sender());
    QString text = textEdit->toPlainText();
    changeValue(typeUtils::textToVariant(text, m_configItem->dataType()));
}

void InputWidgetWrapper::onEditingFinished()
{
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(sender());
    QString text = lineEdit->text();
    changeValue(typeUtils::textToVariant(text, m_configItem->dataType()));
}

void InputWidgetWrapper::onValueChanged(int value)
{
    changeValue(value);
}

void InputWidgetWrapper::onValueChanged(double value)
{
    changeValue(value);
}

void InputWidgetWrapper::onTimeChanged(const QTime & time)
{
    changeValue(time);
}

void InputWidgetWrapper::onDateChanged(const QDate & date)
{
    changeValue(date);
}

void InputWidgetWrapper::onDateTimeChanged(const QDateTime & dateTime)
{
    changeValue(dateTime);
}

void InputWidgetWrapper::onConfigItemModifiedChanged(bool modified)
{
	if (!m_labelName)
		return;
    QPalette::ColorRole role = m_labelName->foregroundRole();
    QPalette palette = m_labelName->palette();
    if (modified)
    {
        palette.setColor(role, Qt::red);
    }
    else
    {
        palette.setColor(role, Qt::black);
    }
    m_labelName->setPalette(palette);
}

void InputWidgetWrapper::onConfigItemValueChanged(const QVariant& value)
{
    updateValue(value);
}

