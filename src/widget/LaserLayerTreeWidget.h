#ifndef LASERLAYERTREEWIDGET_H
#define LASERLAYERTREEWIDGET_H

#include "common/common.h"
#include <QTreeWidget>
#include "scene/LaserLayer.h"

class LaserDocument;
class LaserLayerTreeWidgetPrivate;

class LaserLayerTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit LaserLayerTreeWidget(QWidget* parent = nullptr);
    virtual ~LaserLayerTreeWidget();

    LaserDocument* document() const;
    void setDocument(LaserDocument* doc);

public slots:
    void updateItems();
    void fillLayersTree(QList<LaserLayer*> &layers, const QString& type);

protected:
    virtual void dropEvent(QDropEvent* event);

signals:

private:
    LaserDocument* m_doc;

};

#endif // LASERLAYERTREEWIDGET_H
