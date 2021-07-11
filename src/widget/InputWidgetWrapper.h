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
    explicit InputWidgetWrapper(QWidget* widget, ConfigItem* configItem);
    virtual ~InputWidgetWrapper();

    void setNameLabel(QLabel* label);
    void setDescriptionLabel(QLabel* label);

    QWidget* widget() const;
    void reset();
    void restoreDefault();
    void updateValue(const QVariant& value);
    void changeValue(const QVariant& value);

    bool isModified();
    QVariant value() const;

    static QWidget* createWidget(InputWidgetType widgetType, Qt::Orientation orientation);

signals:
    void valueChanged(const QVariant& newValue);

protected:
    void onTextChanged(const QString& text);
    void onCheckBoxStateChanged(int state);
    void onComboBoxIndexChanged(int index);
    void onTextEditTextChanged();
    void onPlainTextEditTextChanged();
    void onEditingFinished();
    void onValueChanged(int value);
    void onValueChanged(double value);
    void onTimeChanged(const QTime& time);
    void onDateChanged(const QDate& date);
    void onDateTimeChanged(const QDateTime& dateTime);

    void onConfigItemModifiedChanged(bool modified);
    void onConfigItemValueChanged(const QVariant& value);

private:
    QLabel* m_labelName;
    QLabel* m_labelDesc;
    InputWidgetType m_type;
    ConfigItem* m_configItem;
};

#endif // INPUTWIDGETWRAPPER_H