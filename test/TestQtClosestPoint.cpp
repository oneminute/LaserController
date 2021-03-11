#include <QtCore>
#include <QPainterPath>
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QWidget>
#include <QRect>

/*!
    Returns the closest element (position) in \a sourcePath to \a target,
    using \l{QPoint::manhattanLength()} to determine the distances.
*/
QPointF closestPointTo(const QPointF& target, const QPainterPath& sourcePath)
{
    Q_ASSERT(!sourcePath.isEmpty());
    QPointF shortestDistance = sourcePath.elementAt(0) - target;
    qreal shortestLength = shortestDistance.manhattanLength();
    for (int i = 1; i < sourcePath.elementCount(); ++i) {
        const QPointF distance(sourcePath.elementAt(i) - target);
        const qreal length = distance.manhattanLength();
        if (length < shortestLength) {
            shortestDistance = sourcePath.elementAt(i);
            shortestLength = length;
        }
    }
    return shortestDistance;
}

/*!
    Returns \c true if \a projectilePath intersects with any items in \a scene,
    setting \a hitPos to the position of the intersection.
*/
bool hit(const QLineF& ray, const QPainterPath& target, QPointF& hitPos)
{
    // QLineF has normalVector(), which is useful for extending our path to a rectangle.
    // The path needs to be a rectangle, as QPainterPath::intersected() only accounts
    // for intersections between fill areas, which ray doesn't have.
    // Extend the first point in the path out by 1 pixel.
    QLineF startEdge = ray.normalVector();
    startEdge.setLength(1);

    // Swap the points in the line so the normal vector is at the other end of the line.
    QLineF revertLine(ray.p2(), ray.p1());
    QLineF endEdge = revertLine.normalVector();

    // The end point is currently pointing the wrong way; move it to face the same
    // direction as startEdge.
    endEdge.setLength(-1);

    // Now we can create a rectangle from our edges.
    QPainterPath rectPath(startEdge.p1());
    rectPath.lineTo(startEdge.p2());
    rectPath.lineTo(endEdge.p2());
    rectPath.lineTo(endEdge.p1());
    rectPath.lineTo(startEdge.p1());

    // The hit position will be the element (point) of the rectangle that is the
    // closest to where the projectile was fired from.
    QPainterPath intersection = target.intersected(rectPath);
    if (intersection.isEmpty())
        return false;

    hitPos = closestPointTo(ray.p1(), intersection);
    return true;
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QGraphicsView view;
    QGraphicsScene* scene = new QGraphicsScene;
    view.setScene(scene);
    view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QRect targetRect(0, 0, 50, 50);
    QGraphicsItem* item = scene->addRect(targetRect);
    item->setTransformOriginPoint(QPointF(12.5, 12.5));
    //item->setRotation(35);
    //item->setPos(100, 100);
    QPainterPath target;
    target.addRect(targetRect);

    QLineF ray(QPointF(200, 200), QPointF(-100, -100));
    QPainterPath rayPath;
    rayPath.moveTo(ray.p1());
    rayPath.lineTo(ray.p2());
    //projectilePath.lineTo(200, 200);

    QPointF hitPos;
    if (hit(ray, target, hitPos)) {
        scene->addEllipse(hitPos.x() - 2, hitPos.y() - 2, 4, 4, QPen(Qt::red));
        qDebug() << hitPos;
    }

    scene->addPath(rayPath, QPen(Qt::DashLine));
    scene->addText("start")->setPos(ray.p1());
    scene->addText("end")->setPos(ray.p2());

    view.show();

    return app.exec();
}