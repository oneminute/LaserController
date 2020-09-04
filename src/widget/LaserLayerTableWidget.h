#ifndef LASERLAYERTABLEWIDGET_H
#define LASERLAYERTABLEWIDGET_H

#include "common/common.h"
#include <QTableWidget>
#include "scene/LaserLayer.h"

class LaserDocument;

class LaserLayerTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    explicit LaserLayerTableWidget(QWidget* parent = nullptr);
    virtual ~LaserLayerTableWidget();

    LaserDocument* document() const { return m_doc; }
    void setDocument(LaserDocument* doc) { m_doc = doc; }

public slots:
    void updateItems();
    void fillLayers(QList<LaserLayer*> &layers, const QString& type);

protected:

private:
    LaserDocument* m_doc;
};

#endif // LASERLAYERTABLEWIDGET_H