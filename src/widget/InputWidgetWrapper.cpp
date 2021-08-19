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
#include "widget/SmallDiagonalLimitationWidget.h"

class InputWidgetWrapperPrivate
{
    Q_DECLARE_PUBLIC(InputWidgetWrapper)
public:
    InputWidgetWrapperPrivate(InputWidgetWrapper* ptr, ConfigItem* configItem)
        : q_ptr(ptr)
        , labelName(nullptr)
        , labelDesc(nullptr)
        , configItem(configItem)
        , type(IWT_Unknown)
    {

    }

    InputWidgetWrapper* q_ptr;

    QLabel* labelName;
    QLabel* labelDesc;
    InputWidgetType type;
    ConfigItem* configItem;
};

InputWidgetWrapper::InputWidgetWrapper(QWidget* widget, ConfigItem* configItem)
    : QObject(widget)
    , m_ptr(new InputWidgetWrapperPrivate(this, configItem))
{
    Q_D(InputWidgetWrapper);

    d->type = configItem->inputWidgetType();

    switch (d->type)
    {
    case IWT_CheckBox:
    {
        QCheckBox* checkBox = qobject_cast<QCheckBox*>(widget);
        connect(checkBox, &QCheckBox::stateChanged, this, &InputWidgetWrapper::onCheckBoxStateChanged);
        break;
    }
    case IWT_ComboBox:
    {
        QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
        connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InputWidgetWrapper::onComboBoxIndexChanged);
        break;
    }
    case IWT_LineEdit:
    {
        QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget);
        connect(lineEdit, &QLineEdit::editingFinished, this, &InputWidgetWrapper::onEditingFinished);
        break;
    }
    case IWT_TextEdit:
    {
        QTextEdit* textEdit = qobject_cast<QTextEdit*>(widget);
        connect(textEdit, &QTextEdit::textChanged, this, &InputWidgetWrapper::onTextEditTextChanged);
        break;
    }
    case IWT_PlainTextEdit:
    {
        QPlainTextEdit* plainTextEdit = qobject_cast<QPlainTextEdit*>(widget);
        connect(plainTextEdit, &QPlainTextEdit::textChanged, this, &InputWidgetWrapper::onPlainTextEditTextChanged);
        break;
    }
    case IWT_SpinBox:
    {
        QSpinBox* spinBox = qobject_cast<QSpinBox*>(widget);
        connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, QOverload<int>::of(&InputWidgetWrapper::onValueChanged));
        break;
    }
    case IWT_DoubleSpinBox:
    {
        QDoubleSpinBox* doubleSpinBox = qobject_cast<QDoubleSpinBox*>(widget);
        connect(doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, QOverload<double>::of(&InputWidgetWrapper::onValueChanged));
        break;
    }
    case IWT_TimeEdit:
    {
        QTimeEdit* timeEdit = qobject_cast<QTimeEdit*>(widget);
        connect(timeEdit, &QTimeEdit::timeChanged, this, &InputWidgetWrapper::onTimeChanged);
        break;
    }
    case IWT_DateEdit:
    {
        QDateEdit* dateEdit = qobject_cast<QDateEdit*>(widget);
        connect(dateEdit, &QDateEdit::dateChanged, this, &InputWidgetWrapper::onDateChanged);
        break;
    }
    case IWT_DateTimeEdit:
    {
        QDateTimeEdit* dateTimeEdit = qobject_cast<QDateTimeEdit*>(widget);
        connect(dateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, &InputWidgetWrapper::onDateTimeChanged);
        break;
    }
    case IWT_Dial:
    {
        QDial* dial = qobject_cast<QDial*>(widget);
        connect(dial, &QDial::valueChanged, this, QOverload<int>::of(&InputWidgetWrapper::onValueChanged));
        break;
    }
    case IWT_EditSlider:
    {
        EditSlider* editSlider = qobject_cast<EditSlider*>(widget);
        connect(editSlider, &EditSlider::valueChanged, this, QOverload<int>::of(&InputWidgetWrapper::onValueChanged));
        break;
    }
    case IWT_FloatEditSlider:
    {
        FloatEditSlider* floatEditSlider = qobject_cast<FloatEditSlider*>(widget);
        connect(floatEditSlider, &FloatEditSlider::valueChanged, this, QOverload<qreal>::of(&InputWidgetWrapper::onValueChanged));
        break;
    }
    }
    updateValue(d->configItem->value());
    connect(this, &InputWidgetWrapper::valueChanged, d->configItem, &ConfigItem::fromWidget);
    connect(d->configItem, &ConfigItem::modifiedChanged, this, &InputWidgetWrapper::onConfigItemModifiedChanged);
    connect(d->configItem, &ConfigItem::widgetValueChanged, this, &InputWidgetWrapper::onConfigItemValueChanged);
}

InputWidgetWrapper::~InputWidgetWrapper()
{
}

void InputWidgetWrapper::setNameLabel(QLabel* label)
{
    Q_D(InputWidgetWrapper);
    d->labelName = label;
}

void InputWidgetWrapper::setDescriptionLabel(QLabel* label)
{
    Q_D(InputWidgetWrapper);
    d->labelDesc = label;
}

QWidget* InputWidgetWrapper::widget() const
{
    return qobject_cast<QWidget*>(parent());
}

void InputWidgetWrapper::reset()
{
    Q_D(InputWidgetWrapper);
    d->configItem->reset();
    updateValue(d->configItem->value());
}

void InputWidgetWrapper::restoreDefault()
{
    Q_D(InputWidgetWrapper);
    updateValue(d->configItem->defaultValue());
}

void InputWidgetWrapper::updateValue(const QVariant& newValue)
{
    Q_D(InputWidgetWrapper);
    QVariant value = d->configItem->doLoadDataHook(newValue);
    QWidget* widget = qobject_cast<QWidget*>(parent());
    widget->blockSignals(true);
    switch (d->type)
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
        if (value.type() == QVariant::String)
        {
            comboBox->setCurrentText(value.toString());
        }
        else if (value.type() == QVariant::Int)
        {
            comboBox->setCurrentIndex(value.toInt());
        }
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
    Q_D(InputWidgetWrapper);
    if (d->configItem->readOnly())
        return;
    QVariant newValue = d->configItem->doSaveDataHook(value);
    emit valueChanged(newValue);
}

bool InputWidgetWrapper::isModified()
{
    Q_D(InputWidgetWrapper);
    return d->configItem->isModified();
}

QVariant InputWidgetWrapper::value() const
{
    Q_D(const InputWidgetWrapper);
    return d->configItem->value();
}

QWidget* InputWidgetWrapper::createWidget(ConfigItem* item, Qt::Orientation orientation)
{
    QWidget* widget = nullptr;
    switch (item->inputWidgetType())
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
    //case IWT_LimitationWidget:
        //widget = new SmallDiagonalLimitationWidget;
        //break;
    default:
        widget = item->doCreateWidgetHook();
        break;
    }
    return widget;
}

void InputWidgetWrapper::onTextChanged(const QString & text)
{
    Q_D(InputWidgetWrapper);
    changeValue(typeUtils::textToVariant(text, d->configItem->dataType()));
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
    Q_D(InputWidgetWrapper);
    QTextEdit* textEdit = qobject_cast<QTextEdit*>(sender());
    QString text = textEdit->toPlainText();
    changeValue(typeUtils::textToVariant(text, d->configItem->dataType()));
}

void InputWidgetWrapper::onPlainTextEditTextChanged()
{
    Q_D(InputWidgetWrapper);
    QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(sender());
    QString text = textEdit->toPlainText();
    changeValue(typeUtils::textToVariant(text, d->configItem->dataType()));
}

void InputWidgetWrapper::onEditingFinished()
{
    Q_D(InputWidgetWrapper);
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(sender());
    QString text = lineEdit->text();
    changeValue(typeUtils::textToVariant(text, d->configItem->dataType()));
}

void InputWidgetWrapper::onValueChanged(int value)
{
    changeValue(value);
}

void InputWidgetWrapper::onValueChanged(double value)
{
    changeValue(value);
}

void InputWidgetWrapper::onValueChanged(const SmallDiagonalLimitation& value)
{
    QVariant var;
    var.setValue(value);
    changeValue(var);
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
    Q_D(InputWidgetWrapper);
	if (!d->labelName)
		return;
    QPalette::ColorRole role = d->labelName->foregroundRole();
    QPalette palette = d->labelName->palette();
    if (modified)
    {
        palette.setColor(role, Qt::red);
    }
    else
    {
        palette.setColor(role, Qt::black);
    }
    d->labelName->setPalette(palette);
}

void InputWidgetWrapper::onConfigItemValueChanged(const QVariant& value)
{
    updateValue(value);
}

