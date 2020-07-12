#ifndef LASERITEM_H
#define LASERITEM_H

#include <QObject>
#include <QSharedDataPointer>
#include <QGraphicsItem>
#include <QPainterPath>

class LaserDocument;

class LaserItem
{
public:
    LaserItem(LaserDocument* doc);
    //LaserItem(LaserItem* item, QObject* parent = nullptr);
    //LaserItem(const LaserItem& item, QObject* parent = nullptr);
    virtual ~LaserItem();

    //LaserItem& operator=(const LaserItem& other);

protected:
    QGraphicsItem* m_graphicsItem;
    LaserDocument* m_doc;

private:
    //QSharedDataPointer<LaserItemPrivate> d_ptr;
    //friend class LaserItemPrivate;
    Q_DISABLE_COPY(LaserItem);
};

class LaserShapeItem : public LaserItem
{
public:
    LaserShapeItem(LaserDocument* doc);

private:
    Q_DISABLE_COPY(LaserShapeItem);
};

class LaserArcItem : public LaserShapeItem
{
public:
    LaserArcItem(const QPainterPath& path, LaserDocument* doc);
    virtual ~LaserArcItem();

    QPainterPath path() const { return m_path; }
    void setPath(const QPainterPath& path) { m_path = path; }

private:
    QPainterPath m_path;
    Q_DISABLE_COPY(LaserArcItem);
};

class LaserEllipseItem : public LaserShapeItem
{
public:
    LaserEllipseItem(const QRectF bounds, LaserDocument* doc)
        : LaserShapeItem(doc)
        , m_bounds(bounds)
    {}
    virtual ~LaserEllipseItem() {}

    QRectF bounds() const { return m_bounds; }
    void setBounds(const QRectF& bounds) { m_bounds = bounds; }

private:
    QRectF m_bounds;
    Q_DISABLE_COPY(LaserEllipseItem)
};

#endif // LASERITEM_H