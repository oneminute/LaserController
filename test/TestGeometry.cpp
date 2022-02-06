#include <QtCore>
#include <QPainterPath>
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QWidget>
#include <QRect>
#include <QVector2D>
#include <QVector3D>
#include <QTransform>
#include <QDebug>
#include <QtMath>

void testAngles()
{
    QVector2D v(0, 1);
    for (int i = 0; i <= 360; i += 10)
    {
        QVector2D v1, v2;
        v1 = v;

        QTransform tRot;
        tRot.rotate(i);
        v2 = QVector2D(tRot.map(v.toPointF()));
        QVector3D v3 = QVector3D::crossProduct(QVector3D(v2), QVector3D(v1));

        qreal x = QVector2D::dotProduct(v2, v1);
        qreal y = v3.z();
        qreal radians = qAtan2(y, x);
        qDebug() << i << ": " << x << ", " << y;
        qDebug() << i << ", " << qRadiansToDegrees(radians);
    }
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    testAngles();

    return app.exec();
}