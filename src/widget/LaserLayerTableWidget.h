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
    void setDocument(LaserDocument* doc);

public slots:
    void updateItems();
    void laserDocumentClosed();

protected:

signals:
    void layerSelectionChanged(const QString& layerId);

private:
    LaserDocument* m_doc;
};

#endif // LASERLAYERTABLEWIDGET_H