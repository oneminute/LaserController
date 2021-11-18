#ifndef POINTPAIRTABLEWIDGET_H
#define POINTPAIRTABLEWIDGET_H

#include <QTableWidget>
#include "common/common.h"

class PointPairTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    explicit PointPairTableWidget(QWidget* parent = nullptr);
    ~PointPairTableWidget();

    void addNewLine();
    PointPairList pointPairs() const;
    void removeSelected();

    void setLaserPoint(const QPointF& point);
    void setCanvasPoint(const QPointF& point);

    bool isEmpty() const;

private:
    Q_DISABLE_COPY(PointPairTableWidget)
};

#endif // POINTPAIRTABLEWIDGET_H
