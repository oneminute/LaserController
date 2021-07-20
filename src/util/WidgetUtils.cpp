#include "WidgetUtils.h"

#include <QComboBox>

namespace widgetUtils
{

    int findComboBoxIndexByValue(QComboBox* widget, const QVariant& value)
    {
        for (int i = 0; i < widget->count(); i++)
        {
            if (value == widget->itemData(i))
            {
                return i;
            }
        }
        return -1;
    }

    QString findComboBoxStringByValue(QComboBox* widget, const QVariant& value)
    {
        for (int i = 0; i < widget->count(); i++)
        {
            if (value == widget->itemData(i))
            {
                return widget->itemText(i);
            }
        }
        return "";
    }

}
