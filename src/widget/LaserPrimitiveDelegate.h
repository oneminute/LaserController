#ifndef LASERPRIMITIVEDELEGATE_H
#define LASERPRIMITIVEDELEGATE_H

#include "common/common.h"
#include <QItemDelegate>
#include <QStyledItemDelegate>

class LaserPrimitive;

class LaserPrimitiveDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit LaserPrimitiveDelegate(LaserPrimitive* primitive, QWidget* parent = nullptr);
    virtual ~LaserPrimitiveDelegate();

    virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

    LaserPrimitive* primitive() const { return m_primitive; }

protected slots:
    void onBeginEditing();
    void onEndEditing();
    void commitAndCloseFinishRunEditor();

signals:
    void beginEditing() const;
    void endEditing() const;

private:
    bool m_isEditing;
    LaserPrimitive* m_primitive;
};

#endif // LASERPRIMITIVEDELEGATE_H
