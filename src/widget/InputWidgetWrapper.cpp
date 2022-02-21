#include "InputWidgetWrapper.h"

class InputWidgetWrapperPrivate
{
    Q_DECLARE_PUBLIC(InputWidgetWrapper)
public:
    InputWidgetWrapperPrivate(InputWidgetWrapper* ptr, ConfigItem* configItem)
        : q_ptr(ptr)
        , labelName(nullptr)
        , labelDesc(nullptr)
        , configItem(configItem)
        , type(IWT_Custom)
    {

    }

    InputWidgetWrapper* q_ptr;

    QLabel* labelName;
    QLabel* labelDesc;
    InputWidgetType type;
    ConfigItem* configItem;
    StoreStrategy storeStrategy;
};

InputWidgetWrapper::InputWidgetWrapper(QWidget* widget, ConfigItem* configItem)
    : QObject(widget)
    , m_ptr(new InputWidgetWrapperPrivate(this, configItem))
{
    Q_D(InputWidgetWrapper);
    Q_ASSERT(widget);
    setParent(widget);

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
        connect(floatEditSlider, &FloatEditSlider::valueChanged, this, &InputWidgetWrapper::onFloatEditSliderValueChanged);
        break;
    }
    case IWT_Vector2DWidget:
    {
        Vector2DWidget* vector2DWidget = qobject_cast<Vector2DWidget*>(widget);
        connect(vector2DWidget, &Vector2DWidget::valueChanged, this, QOverload<qreal, qreal>::of(&InputWidgetWrapper::onVector2DChanged));
        break;
    }
    case IWT_Vector3DWidget:
    {
        Vector3DWidget* vector3DWidget = qobject_cast<Vector3DWidget*>(widget);
        connect(vector3DWidget, &Vector3DWidget::valueChanged, this, QOverload<qreal, qreal, qreal>::of(&InputWidgetWrapper::onVector3DChanged));
        break;
    }
    }
    //configItem->blockSignals(true);
    if (d->configItem->name() == "xStepLength")
        qLogD << "debugging " << d->configItem->name();
    updateWidgetValue(d->configItem->value(), nullptr);
    //configItem->blockSignals(false);

    connect(d->configItem, &ConfigItem::modifiedChanged, this, &InputWidgetWrapper::onConfigItemModifiedChanged);
    connect(d->configItem, &ConfigItem::valueChanged, this, &InputWidgetWrapper::onConfigItemValueChanged);
    connect(d->configItem, &ConfigItem::dirtyValueChanged, this, &InputWidgetWrapper::onConfigItemDirtyValueChanged);
    connect(d->configItem, &ConfigItem::lazyValueChanged, this, &InputWidgetWrapper::onConfigItemValueChanged);
    connect(d->configItem, &ConfigItem::enabledChanged, this, &InputWidgetWrapper::onConfigItemEnabledChanged);
}

InputWidgetWrapper::~InputWidgetWrapper()
{
}

void InputWidgetWrapper::setNameLabel(QLabel* label)
{
    Q_D(InputWidgetWrapper);
    d->labelName = label;
    retranslate();
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

void InputWidgetWrapper::updateWidgetValue(const QVariant& newValue, void* senderPtr)
{
    Q_D(InputWidgetWrapper);
    if (senderPtr == this)
        return;
    //QVariant value = d->configItem->doValueToWidgetHook(newValue);
    QWidget* widget = qobject_cast<QWidget*>(parent());
    //widget->blockSignals(true);
    // for debugging
    if (d->configItem->name() == "xStepLength")
        qLogD << "debugging " << d->configItem->name();
    bool done = d->configItem->doUpdateWidgetValueHook(widget, newValue);
    if (!done)
    {
        switch (d->type)
        {
        case IWT_CheckBox:
        {
            QCheckBox* checkBox = qobject_cast<QCheckBox*>(widget);
            Qt::CheckState checked;
            if (d->configItem->dataType() == DT_INT)
                checked = newValue.toInt() == 1 ? Qt::Checked : Qt::Unchecked;
            else
                checked = newValue.toBool() ? Qt::Checked : Qt::Unchecked;
            checkBox->setCheckState(checked);
            break;
        }
        case IWT_ComboBox:
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
            if (newValue.type() == QVariant::String)
            {
                comboBox->setCurrentText(newValue.toString());
            }
            else if (newValue.type() == QVariant::Int)
            {
                comboBox->setCurrentIndex(newValue.toInt());
            }
            break;
        }
        case IWT_LineEdit:
        {
            QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget);
            lineEdit->setText(newValue.toString());
            break;
        }
        case IWT_TextEdit:
        {
            QTextEdit* textEdit = qobject_cast<QTextEdit*>(widget);
            textEdit->setText(newValue.toString());
            break;
        }
        case IWT_PlainTextEdit:
        {
            QPlainTextEdit* plainTextEdit = qobject_cast<QPlainTextEdit*>(widget);
            plainTextEdit->setPlainText(newValue.toString());
            break;
        }
        case IWT_SpinBox:
        {
            QSpinBox* spinBox = qobject_cast<QSpinBox*>(widget);
            spinBox->setValue(newValue.toInt());
            break;
        }
        case IWT_DoubleSpinBox:
        {
            QDoubleSpinBox* doubleSpinBox = qobject_cast<QDoubleSpinBox*>(widget);
            doubleSpinBox->setValue(newValue.toDouble());
            break;
        }
        case IWT_TimeEdit:
        {
            QTimeEdit* timeEdit = qobject_cast<QTimeEdit*>(widget);
            timeEdit->setTime(newValue.toTime());
            break;
        }
        case IWT_DateEdit:
        {
            QDateEdit* dateEdit = qobject_cast<QDateEdit*>(widget);
            dateEdit->setDate(newValue.toDate());
            break;
        }
        case IWT_DateTimeEdit:
        {
            QDateTimeEdit* dateTimeEdit = qobject_cast<QDateTimeEdit*>(widget);
            dateTimeEdit->setDateTime(newValue.toDateTime());
            break;
        }
        case IWT_Dial:
        {
            QDial* dial = qobject_cast<QDial*>(widget);
            dial->setValue(newValue.toInt());
            break;
        }
        case IWT_EditSlider:
        {
            EditSlider* editSlider = qobject_cast<EditSlider*>(widget);
            editSlider->setValue(newValue.toInt());
            break;
        }
        case IWT_FloatEditSlider:
        {
            FloatEditSlider* editSlider = qobject_cast<FloatEditSlider*>(widget);
            if (d->configItem->dataType() == DT_REAL ||
                d->configItem->dataType() == DT_FLOAT ||
                d->configItem->dataType() == DT_DOUBLE)
                editSlider->setValue(newValue.toReal());
            else if (d->configItem->dataType() == DT_INT)
                editSlider->setIntValue(newValue.toInt());
            break;
        }
        case IWT_Vector2DWidget:
        {
            Vector2DWidget* vector2DWidget = qobject_cast<Vector2DWidget*>(widget);
            vector2DWidget->setValue(newValue.toPointF());
            break;
        }
        case IWT_Vector3DWidget:
        {
            Vector3DWidget* vector3DWidget = qobject_cast<Vector3DWidget*>(widget);
            vector3DWidget->setValue(newValue.value<QVector3D>());
            break;
        }
        }
    }
    //widget->blockSignals(false);
}

void InputWidgetWrapper::modifyConfigItemValue(const QVariant& value)
{
    Q_D(InputWidgetWrapper);
    if (d->configItem->name() == "xStepLength")
        qLogD << "debugging " << d->configItem->name();
    QWidget* widget = qobject_cast<QWidget*>(parent());
    QVariant newValue = d->configItem->doValueFromWidgetHook(widget, value);
    d->configItem->setValue(newValue, d->storeStrategy, this);
}

QVariant InputWidgetWrapper::value() const
{
    Q_D(const InputWidgetWrapper);
    return d->configItem->value();
}

StoreStrategy InputWidgetWrapper::storeStrategy() const
{
    Q_D(const InputWidgetWrapper);
    return d->storeStrategy;
}

void InputWidgetWrapper::setStoreStrategy(StoreStrategy storeStrategy)
{
    Q_D(InputWidgetWrapper);
    d->storeStrategy = storeStrategy;
}

void InputWidgetWrapper::retranslate()
{
    Q_D(InputWidgetWrapper);
    d->labelName->setText(d->configItem->title());
    d->labelName->setToolTip(d->configItem->description());
}

void InputWidgetWrapper::updateLabelColor()
{
    Q_D(InputWidgetWrapper);
	if (!d->labelName)
		return;
    QPalette::ColorRole role = d->labelName->foregroundRole();
    QPalette palette = d->labelName->palette();
    if (d->configItem->isModified())
    {
        palette.setColor(role, Qt::blue);
    }
    else if (d->configItem->isDirty())
    {
        palette.setColor(role, Qt::red);
    }
    else
    {
        palette.setColor(role, Qt::black);
    }
    d->labelName->setPalette(palette);
}

ConfigItem* InputWidgetWrapper::configItem() const
{
    Q_D(const InputWidgetWrapper);
    return d->configItem;
}

void InputWidgetWrapper::setEnabled(bool enabled)
{
    Q_D(InputWidgetWrapper);
    qobject_cast<QWidget*>(parent())->setEnabled(enabled);
}

void InputWidgetWrapper::onTextChanged(const QString & text)
{
    Q_D(InputWidgetWrapper);
    modifyConfigItemValue(typeUtils::stringToVariant(text, d->configItem->dataType()));
}

void InputWidgetWrapper::onCheckBoxStateChanged(int state)
{
    Q_D(InputWidgetWrapper);
    QCheckBox* checkBox = qobject_cast<QCheckBox*>(sender());
    QVariant value = state == Qt::Checked;
    if (d->configItem->dataType() == DT_INT)
        value = state == Qt::Checked ? 1 : 0;
    modifyConfigItemValue(value);
}

void InputWidgetWrapper::onComboBoxIndexChanged(int index)
{
    QComboBox* comboBox = qobject_cast<QComboBox*>(sender());
    modifyConfigItemValue(comboBox->currentData());
}

void InputWidgetWrapper::onTextEditTextChanged()
{
    Q_D(InputWidgetWrapper);
    QTextEdit* textEdit = qobject_cast<QTextEdit*>(sender());
    QString text = textEdit->toPlainText();
    modifyConfigItemValue(typeUtils::stringToVariant(text, d->configItem->dataType()));
}

void InputWidgetWrapper::onPlainTextEditTextChanged()
{
    Q_D(InputWidgetWrapper);
    QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(sender());
    QString text = textEdit->toPlainText();
    modifyConfigItemValue(typeUtils::stringToVariant(text, d->configItem->dataType()));
}

void InputWidgetWrapper::onEditingFinished()
{
    Q_D(InputWidgetWrapper);
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(sender());
    QString text = lineEdit->text();
    modifyConfigItemValue(typeUtils::stringToVariant(text, d->configItem->dataType()));
}

void InputWidgetWrapper::onFloatEditSliderValueChanged(qreal value)
{
    Q_D(InputWidgetWrapper);
    QVariant var;
    FloatEditSlider* editSlider = qobject_cast<FloatEditSlider*>(sender());
    if (d->configItem->dataType() == DT_REAL ||
        d->configItem->dataType() == DT_FLOAT ||
        d->configItem->dataType() == DT_DOUBLE)
        var = value;
    else if (d->configItem->dataType() == DT_INT)
    {
        if (d->configItem->name() == "xStepLength")
            qLogD << "debugging " << d->configItem->name();
        var = editSlider->intValue();
    }
    modifyConfigItemValue(var);
}

void InputWidgetWrapper::onValueChanged(int value)
{
    modifyConfigItemValue(value);
}

void InputWidgetWrapper::onValueChanged(double value)
{
    modifyConfigItemValue(value);
}

void InputWidgetWrapper::onValueChanged(const SmallDiagonalLimitation& value)
{
    QVariant var;
    var.setValue(value);
    modifyConfigItemValue(var);
}

void InputWidgetWrapper::onTimeChanged(const QTime & time)
{
    modifyConfigItemValue(time);
}

void InputWidgetWrapper::onDateChanged(const QDate & date)
{
    modifyConfigItemValue(date);
}

void InputWidgetWrapper::onDateTimeChanged(const QDateTime & dateTime)
{
    modifyConfigItemValue(dateTime);
}

void InputWidgetWrapper::onVector2DChanged(qreal x, qreal y)
{
    modifyConfigItemValue(QPointF(x, y));
}

void InputWidgetWrapper::onVector3DChanged(qreal x, qreal y, qreal z)
{
    modifyConfigItemValue(QVariant::fromValue<QVector3D>(QVector3D(x, y, z)));
}

void InputWidgetWrapper::onConfigItemModifiedChanged(bool modified)
{
    updateLabelColor();
    emit updated();
}

void InputWidgetWrapper::onConfigItemValueChanged(const QVariant& value, void* senderPtr)
{
    if (senderPtr == this)
        return;
    updateLabelColor();
    updateWidgetValue(value, senderPtr);
    emit updated();
}

void InputWidgetWrapper::onConfigItemDirtyValueChanged(const QVariant& value, void* senderPtr)
{
    if (senderPtr == this)
        return;
    updateLabelColor();
    updateWidgetValue(value, senderPtr);
    emit updated();
}

void InputWidgetWrapper::onConfigItemLazyValueChanged(const QVariant& value, void* senderPtr)
{
    if (senderPtr == this)
        return;
    updateLabelColor();
    updateWidgetValue(value, senderPtr);
    emit updated();
}

void InputWidgetWrapper::onConfigItemEnabledChanged(bool enabled)
{
    Q_D(InputWidgetWrapper);
    QWidget* widget = qobject_cast<QWidget*>(parent());
    widget->setEnabled(enabled);
}


