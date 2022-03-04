#ifndef UTILS_H
#define UTILS_H

#include "common/common.h"

#include <QObject>
#include <QFrame>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QGraphicsItem>
#include <QPointF>

#include "laser/LaserPointList.h"
#include "laser/LaserLineList.h"
#include "common/common.h"

class LaserPrimitive;

namespace utils
{
    QString createUUID(const QString& prefix = "");

    int parsePortName(const QString& name);

    QFrame* createSeparator(int width = 0, int height = 0, QFrame::Shape shape = QFrame::VLine, QFrame::Shadow shadow = QFrame::Sunken);

    QVector3D putToQuadrant(const QVector3D& pos, QUADRANT quadrant);

    QVector3D limitToLayout(const QVector3D& pos, int quadrant, float width, float height);

	bool checkTwoPointEqueal(const QPointF & point1, const QPointF & point2, float scop = 0.00001f);

    bool fuzzyEquals(const QPointF& pt1, const QPointF& pt2);

    bool fuzzyEquals(qreal a, qreal b, qreal limit = 0.001);

    bool fuzzyCompare(const QPointF& p1, const QPointF& p2);

	void sceneTransformToItemTransform(QTransform sceneTransform, QGraphicsItem* item);
	
    quint32 factorial(quint32 n);

    QPointF center(const QVector<QPointF>& points);

    LaserPoint center(const LaserPointList& points);

    void boundingRect(const QList<QGraphicsItem*>& primitives, QRect& bounding, QRect& boundingAcc = QRect(), bool exludeUnexport = true);

    void boundingRect(const QList<LaserPrimitive*>& primitives, QRect& bounding, QRect& boundingAcc = QRect(), bool exludeUnexport = true);

    void boundingRect(const QSet<LaserPrimitive*>& primitives, QRect& bounding, QRect& boundingAcc = QRect(), bool exludeUnexport = true);

    bool computeBoundingRect(LaserPrimitive* primitive, QRect& bounding, int& count, QRect& boundingAcc = QRect(), bool exludeUnexport = true);

    LaserLineListList interLines(const QPainterPath& path, qreal rowInterval = 1);

    RELATION determineRelationship(const QPainterPath& a, const QPainterPath& b);

    RELATION determineRelationship(const QRectF& a, const QRectF& b);

    QTransform leastSquare4d(const PointPairList& pointPairs, const QPointF& offset = QPointF());

    QTransform leastSquare6d(const PointPairList& pointPairs, const QPointF& offset = QPointF());

    QTransform transformFrom2Points(const PointPairList& pointPairs);

    void rectEdges(QRectF rect, QList<QLineF>& edges);

    void warning(const QString& title, const QString& msg, QWidget* parent = nullptr);
}

#endif // UTILS_H
