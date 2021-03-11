#include <QtCore>
#include <QPainterPath>
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QWidget>
#include <QRect>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QGraphicsView view;
    QGraphicsScene* scene = new QGraphicsScene;
    view.setScene(scene);
    view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QPainterPath rectPath;
    rectPath.addEllipse(QPointF(0, 0), 200, 200);

    for (int i = 0; i < rectPath.elementCount(); i++)
    {
        QPainterPath::Element e = rectPath.elementAt(i);
        qDebug() << e << e.type;

        QPoint lt(e.x - 3, e.y - 3);
        QPoint rb(e.x + 3, e.y + 3);
        scene->addEllipse(QRectF(lt, rb), QPen(Qt::red));
    }

    scene->addPath(rectPath);

    view.show();

    return app.exec();
}