#include "LaserPrimitiveDelegate.h"

#include <QApplication>
#include <QLineEdit>
#include "widget/FinishRunWidget.h"

LaserPrimitiveDelegate::LaserPrimitiveDelegate(LaserPrimitive* primitive, QWidget* parent)
    : QStyledItemDelegate(parent)
    , m_isEditing(false)
    , m_primitive(primitive)
{
    connect(this, &LaserPrimitiveDelegate::beginEditing, this, &LaserPrimitiveDelegate::onBeginEditing);
    connect(this, &LaserPrimitiveDelegate::endEditing, this, &LaserPrimitiveDelegate::onEndEditing);
}

LaserPrimitiveDelegate::~LaserPrimitiveDelegate()
{

}

QWidget * LaserPrimitiveDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    PropertyType type = static_cast<PropertyType>(index.data(WidgetUserData::WUD_PropertyType).toInt());
    PropertyEditor editorType = static_cast<PropertyEditor>(index.data(WidgetUserData::WUD_PropertyEditor).toInt());
    QVariant value = index.data(WidgetUserData::WUD_PropertyValue);

    QWidget* editor = nullptr;
    if (editorType == PropertyEditor::PE_LineEdit)
    {
        editor = new QLineEdit(parent);
    }
    else if (editorType == PropertyEditor::PE_FinishRunWidget)
    {
        editor = new FinishRunWidget(parent);

        connect((FinishRunWidget*)editor, &FinishRunWidget::closed, this, &LaserPrimitiveDelegate::commitAndCloseFinishRunEditor);
        //connect((FinishRunWidget*)editor, &FinishRunWidget::closed, this, &LaserPrimitiveDelegate::onEndEditing);
    }
    return editor;
}

void LaserPrimitiveDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    PropertyType type = static_cast<PropertyType>(index.data(WidgetUserData::WUD_PropertyType).toInt());
    PropertyEditor editorType = static_cast<PropertyEditor>(index.data(WidgetUserData::WUD_PropertyEditor).toInt());
    QVariant value = index.data(WidgetUserData::WUD_PropertyValue);
    QStyleOptionViewItem itemOption(option);
    initStyleOption(&itemOption, index);

    bool modified = false;
    if (editorType == PropertyEditor::PE_FinishRunWidget)
    {
        if (m_isEditing)
        {
            itemOption.text = "";
            modified = true;
        }
        else
        {
            FinishRun finishRun = value.value<FinishRun>();
            itemOption.text = finishRun.toString();
            modified = true;
        }
    }
    
    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &itemOption, painter, nullptr);
    if (!modified)
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize LaserPrimitiveDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    PropertyType type = static_cast<PropertyType>(index.data(WidgetUserData::WUD_PropertyType).toInt());
    PropertyEditor editorType = static_cast<PropertyEditor>(index.data(WidgetUserData::WUD_PropertyEditor).toInt());
    QVariant value = index.data(WidgetUserData::WUD_PropertyValue);

    if (editorType == PropertyEditor::PE_FinishRunWidget)
    {
        return QSize(300, 66);
    }

    return QStyledItemDelegate::sizeHint(option, index);
}

void LaserPrimitiveDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
    PropertyType type = static_cast<PropertyType>(index.data(WidgetUserData::WUD_PropertyType).toInt());
    PropertyEditor editorType = static_cast<PropertyEditor>(index.data(WidgetUserData::WUD_PropertyEditor).toInt());
    QVariant value = index.data(WidgetUserData::WUD_PropertyValue);

    if (editorType == PropertyEditor::PE_FinishRunWidget)
    {
        qDebug() << value;
        FinishRunWidget* finishRunWidget = qobject_cast<FinishRunWidget*>(editor);
        finishRunWidget->setFinishRun(value.value<FinishRun>());
        emit beginEditing();
    }

    QStyledItemDelegate::setEditorData(editor, index);
}

void LaserPrimitiveDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const
{
    PropertyType type = static_cast<PropertyType>(index.data(WidgetUserData::WUD_PropertyType).toInt());
    PropertyEditor editorType = static_cast<PropertyEditor>(index.data(WidgetUserData::WUD_PropertyEditor).toInt());
    QVariant value = index.data(WidgetUserData::WUD_PropertyValue);

    if (editorType == PropertyEditor::PE_FinishRunWidget)
    {
        qDebug() << value;
        FinishRunWidget* finishRunWidget = qobject_cast<FinishRunWidget*>(editor);
        model->setData(index, QVariant::fromValue<FinishRun>(finishRunWidget->finishRun()), WidgetUserData::WUD_PropertyValue);
        emit endEditing();
    }

    QStyledItemDelegate::setEditorData(editor, index);
}

void LaserPrimitiveDelegate::onBeginEditing()
{
    m_isEditing = true;
}

void LaserPrimitiveDelegate::onEndEditing()
{
    qDebug() << "editor closed";
    m_isEditing = false;
}

void LaserPrimitiveDelegate::commitAndCloseFinishRunEditor()
{
    FinishRunWidget *editor = qobject_cast<FinishRunWidget *>(sender());
    qDebug() << editor;
    if (editor)
    {
        emit commitData(editor);
        emit closeEditor(editor);
    }
}
