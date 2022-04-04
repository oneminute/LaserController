#ifndef WIDGETUTILS_H
#define WIDGETUTILS_H

#include "common/common.h"

class QComboBox;

namespace widgetUtils
{
    int findComboBoxIndexByValue(QComboBox* widget, const QVariant& value);

    QString findComboBoxStringByValue(QComboBox* widget, const QVariant& value);

    void showWarningMessage(QWidget* parentWnd, const QString& title, const QString& msg);
}

#endif // WIDGETUTILS_H
