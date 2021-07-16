#ifndef WIDGETUTILS_H
#define WIDGETUTILS_H

#include "common/common.h"

class QComboBox;

namespace widgetUtils
{
    int findComboBoxItemByValue(QComboBox* widget, const QVariant& value);
}

#endif // WIDGETUTILS_H
