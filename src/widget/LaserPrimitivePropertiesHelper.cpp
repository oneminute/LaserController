#include "LaserPrimitivePropertiesHelper.h"

#include <QTableWidget>
#include <QTableWidgetItem>
#include "scene/LaserPrimitive.h"
#include "widget/LaserPrimitiveDelegate.h"

LaserPrimitivePropertiesHelper::LaserPrimitivePropertiesHelper()
    : m_table(nullptr)
    , m_primitive(nullptr)
{
}

void LaserPrimitivePropertiesHelper::resetProperties(LaserPrimitive * primitive, QTableWidget* table)
{
    if (!table)
        return;

    if (!primitive)
        return;

    m_primitive = primitive;
    m_table = table;

    m_table->setRowCount(2);
    m_table->setItemDelegateForColumn(1, new LaserPrimitiveDelegate(primitive));

    QTableWidgetItem* nameLabelItem = new QTableWidgetItem;
    nameLabelItem->setText(QObject::tr("Name"));
    nameLabelItem->setFlags(nameLabelItem->flags() & ~Qt::ItemIsEditable);
    QTableWidgetItem* nameItem = new QTableWidgetItem;
    nameItem->setText(m_primitive->name());
    nameItem->setData(WidgetUserData::WUD_PropertyValue, m_primitive->name());
    nameItem->setData(WidgetUserData::WUD_PropertyType, PropertyType::PT_Text);
    nameItem->setData(WidgetUserData::WUD_PropertyEditor, PropertyEditor::PE_LineEdit);
    m_table->setItem(0, 0, nameLabelItem);
    m_table->setItem(0, 1, nameItem);

    /*QTableWidgetItem* finishRunLabelItem = new QTableWidgetItem;
    finishRunLabelItem->setText(QObject::tr("Finish Run"));
    finishRunLabelItem->setFlags(nameLabelItem->flags() & ~Qt::ItemIsEditable);
    QTableWidgetItem* finishRunItem = new QTableWidgetItem;
    finishRunItem->setText(m_primitive->finishRun().toString());
    finishRunItem->setData(WidgetUserData::WUD_PropertyValue, QVariant::fromValue<FinishRun>(m_primitive->finishRun()));
    finishRunItem->setData(WidgetUserData::WUD_PropertyType, PropertyType::PT_FinishRun);
    finishRunItem->setData(WidgetUserData::WUD_PropertyEditor, PropertyEditor::PE_FinishRunWidget);
    m_table->setItem(1, 0, finishRunLabelItem);
    m_table->setItem(1, 1, finishRunItem);
    m_table->resizeRowToContents(1);*/
}
