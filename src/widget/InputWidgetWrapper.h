#ifndef INPUTWIDGETWRAPPER_H
#define INPUTWIDGETWRAPPER_H

#include <QObject>
#include <QVariant>
#include <QWidget>
#include "common/Config.h"

class QComboBox;
class QLineEdit;
class QTextEdit;
class QPlainTextEdit;
class QSpinBox;
class QDoubleSpinBox;
class QLabel;

class InputWidgetWrapper : public QObject
{
    Q_OBJECT
public:
    enum WidgetType
    {
        WT_Unknown,
        WT_ComboBox,
        WT_LineEdit,
        WT_TextEdit,
        WT_PlainTextEdit,
        WT_SpinBox,
        WT_DoubleSpinBox,
        WT_TimeEdit,
        WT_DateEdit,
        WT_DateTimeEdit,
        WT_Dial,
        WT_EditSlider
    };

    explicit InputWidgetWrapper(QWidget* widget, Config::ConfigItem* configItem, QLabel* labelName, QLabel* labelDesc);
    virtual ~InputWidgetWrapper();

    void updateConfigItem();
    void restore();
    void restoreDefault();

    bool isModified();
    QVariant value()
    {
        return m_value;
    }

signals:
    void valueChanged(const QVariant& newValue);

protected:
    void onTextChanged(const QString& text);
    void onTextEditTextChanged();
    void onPlainTextEditTextChanged();
    void onValueChanged(int value);
    void onValueChanged(double value);
    void onTimeChanged(const QTime& time);
    void onDateChanged(const QDate& date);
    void onDateTimeChanged(const QDateTime& dateTime);

    void setValue(const QVariant& value);

private:
    QLabel* m_labelName;
    QLabel* m_labelDesc;
    WidgetType m_type;
    QVariant m_value;
    Config::ConfigItem* m_configItem;
    bool m_modified;
};

#endif // INPUTWIDGETWRAPPER_H