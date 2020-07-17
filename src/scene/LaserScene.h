#ifndef LASERSCENE_H
#define LASERSCENE_H

#include <QObject>
#include <QGraphicsScene>

class QPaintEvent;
class LaserViewer;
class LaserDocument;

class LaserScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit LaserScene(QObject* parent = nullptr);
    ~LaserScene();

    void updateDocument(LaserDocument* doc);
    void clearDocument(bool delDoc = false);

    LaserDocument* document() { return m_doc; }

private:

private:
    LaserDocument* m_doc;
    QGraphicsRectItem* m_background;
};

#endif // LASERSCENE_H