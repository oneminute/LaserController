#include "OutlineTreeWidget.h"

class OutlineTreeWidgetPrivate
{
    Q_DECLARE_PUBLIC(OutlineTreeWidget)
public:
    OutlineTreeWidgetPrivate(OutlineTreeWidget* ptr)
        : q_ptr(ptr)
    { }

    OutlineTreeWidget* q_ptr;
};

OutlineTreeWidget::OutlineTreeWidget(QWidget* parent)
    : QTreeWidget(parent)
{

}

OutlineTreeWidget::~OutlineTreeWidget()
{
}
