#ifndef OUTLINETREEWIDGET_H
#define OUTLINETREEWIDGET_H

#include <QTreeWidget.h>

class OutlineTreeWidgetPrivate;

class OutlineTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit OutlineTreeWidget(QWidget* parent = nullptr);
    ~OutlineTreeWidget();

private:
    QScopedPointer<OutlineTreeWidgetPrivate> d_ptr;

    Q_DISABLE_COPY(OutlineTreeWidget)
    Q_DECLARE_PRIVATE(OutlineTreeWidget)
};

#endif // OUTLINETREEWIDGET_H