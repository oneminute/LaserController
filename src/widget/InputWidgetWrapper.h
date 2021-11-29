#ifndef INPUTWIDGETWRAPPER_H
#define INPUTWIDGETWRAPPER_H

#include <QObject>
#include <QVariant>
#include <QWidget>
#include "common/Config.h"
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
#include "widget/Vector2DWidget.h"

class QComboBox;
class QLineEdit;
class QTextEdit;
class QPlainTextEdit;
class QSpinBox;
class QDoubleSpinBox;
class QLabel;

class InputWidgetWrapperPrivate;
class InputWidgetWrapper : public QObject
{
    Q_OBJECT
public:
    explicit InputWidgetWrapper(QWidget* widget, ConfigItem* configItem);
    ~InputWidgetWrapper();

    void setNameLabel(QLabel* label);
    void setDescriptionLabel(QLabel* label);

    QWidget* widget() const;
    void updateWidgetValue(const QVariant& value, void* senderPtr);
    void modifyConfigItemValue(const QVariant& value);

    QVariant value() const;

    StoreStrategy storeStrategy() const;
    void setStoreStrategy(StoreStrategy storeStrategy);

    void retranslate();

    void updateLabelColor();

    ConfigItem* configItem() const;

    template<typename T = QWidget*>
    static T createWidget(ConfigItem* item, Qt::Orientation orientation = Qt::Horizontal)
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
        case IWT_Vector2DWidget:
            widget = new Vector2DWidget;
            break;
        default:
            widget = item->doCreateWidgetHook();
            break;
        }
        return qobject_cast<T>(widget);
    }

public slots:
    void setEnabled(bool enabled);
    void onTextChanged(const QString& text);
    void onCheckBoxStateChanged(int state);
    void onComboBoxIndexChanged(int index);
    void onTextEditTextChanged();
    void onPlainTextEditTextChanged();
    void onEditingFinished();
    void onFloatEditSliderValueChanged(qreal value);
    void onValueChanged(int value);
    void onValueChanged(double value);
    void onValueChanged(const SmallDiagonalLimitation& value);
    void onTimeChanged(const QTime& time);
    void onDateChanged(const QDate& date);
    void onDateTimeChanged(const QDateTime& dateTime);
    void onVector2DChanged(qreal x, qreal y);

    void onConfigItemModifiedChanged(bool modified);
    void onConfigItemValueChanged(const QVariant& value, void* senderPtr);
    void onConfigItemDirtyValueChanged(const QVariant& value, void* senderPtr);
    void onConfigItemLazyValueChanged(const QVariant& value, void* senderPtr);

signals:
    void updated();

private:
    QScopedPointer<InputWidgetWrapperPrivate> m_ptr;
    Q_DECLARE_PRIVATE_D(m_ptr, InputWidgetWrapper)
    Q_DISABLE_COPY(InputWidgetWrapper)
};

#endif // INPUTWIDGETWRAPPER_H