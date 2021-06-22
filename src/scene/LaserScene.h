#ifndef LASERSCENE_H
#define LASERSCENE_H

#include <QObject>
#include <QGraphicsScene>

class QPaintEvent;
class QPushButton;

class LaserViewer;
class LaserDocument;
class LaserPrimitive;
class LaserPrimitiveGroup;

class LaserScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit LaserScene(QObject* parent = nullptr);
    ~LaserScene();

    void updateDocument(LaserDocument* doc);
    void clearDocument(bool delDoc = false);
    void addLaserPrimitive(LaserPrimitive* primitive);
	QGraphicsRectItem* backgroundItem() { return m_background; }

    LaserDocument* document() { return m_doc; }

    QList<LaserPrimitive*> selectedPrimitives() const;

	LaserPrimitiveGroup *createItemGroup(const QList<LaserPrimitive*> &items);
	void destroyItemGroup(LaserPrimitiveGroup *group);

private:

private:
    LaserDocument* m_doc;
    QGraphicsRectItem* m_background;
};

#endif // LASERSCENE_H