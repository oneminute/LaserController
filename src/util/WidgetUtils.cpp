#include "WidgetUtils.h"

#include <QComboBox>
#include <QMessageBox>

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

    void showWarningMessage(QWidget* parentWnd, const QString& title, const QString& msg)
    {
        QMessageBox dlg(QMessageBox::Icon::Warning, title, msg, QMessageBox::Ok, parentWnd);
        dlg.setButtonText(QMessageBox::Ok, QObject::tr("Ok"));
    }

    void showInfoMessage(QWidget* parentWnd, const QString& title, const QString& msg)
    {
        QMessageBox dlg(QMessageBox::Icon::Information, title, msg, QMessageBox::Ok, parentWnd);
        dlg.setButtonText(QMessageBox::Ok, QObject::tr("Ok"));
    }
}
