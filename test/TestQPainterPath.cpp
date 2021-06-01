#include <QtCore>
#include <QPainterPath>
#

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    QPainterPath path;
    path.addEllipse(QPointF(0, 0), 10, 10);
    
    for (int i = 0; i < path.elementCount(); i++)
    {
        QPainterPath::Element element = path.elementAt(i);
        qDebug() << "path1: " << element.type << element;
    }

    QPainterPath path2;
    path2.moveTo(-5, 0);
    path2.cubicTo(QPointF(-2.5, 8), QPointF(2.5, 8), QPointF(5, 0));
    qDebug();
    for (int i = 0; i < path2.elementCount(); i++)
    {
        QPainterPath::Element element = path2.elementAt(i);
        qDebug() << "path2: " << element.type << element;
    }

    return 0;
}