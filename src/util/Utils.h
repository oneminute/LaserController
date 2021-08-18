#ifndef UTILS_H
#define UTILS_H

#include "common/common.h"

#include <QObject>
#include <QFrame>
#include <QVector3D>
#include <QVector2D>
#include <QGraphicsItem>

namespace utils
{
    QString createUUID(const QString& prefix = "");

    int parsePortName(const QString& name);

    QFrame* createSeparator(int width = 0, int height = 0, QFrame::Shape shape = QFrame::VLine, QFrame::Shadow shadow = QFrame::Sunken);

    QVector3D putToQuadrant(const QVector3D& pos, QUADRANT quadrant);

    void limitToLayout(QVector3D& pos, QUADRANT quadrant, float width, float height);

	bool checkTwoPointEqueal(const QPointF & point1, const QPointF & point2, float scop = 0.00001f);

    bool fuzzyEquals(const QPointF& pt1, const QPointF& pt2);

	void sceneTransformToItemTransform(QTransform sceneTransform, QGraphicsItem* item);
	
    quint32 factorial(quint32 n);
}

#endif // UTILS_H
