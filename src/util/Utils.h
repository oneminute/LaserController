#ifndef UTILS_H
#define UTILS_H

#include "common/common.h"

#include <QObject>
#include <QFrame>
#include <QVector3D>
#include <QVector2D>

namespace utils
{
    QString createUUID(const QString& prefix = "");

    int parsePortName(const QString& name);

    QFrame* createSeparator(int width = 0, int height = 0, QFrame::Shape shape = QFrame::VLine, QFrame::Shadow shadow = QFrame::Sunken);

    QVector3D putToQuadrant(const QVector3D& pos, QUADRANT quadrant);

    void limitToLayout(QVector3D& pos, QUADRANT quadrant, float width, float height);

	bool checkTwoPointEqueal(const QPointF & point1, const QPointF & point2);

}

#endif // UTILS_H
