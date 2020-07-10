#ifndef LASERITEM_H
#define LASERITEM_H

#include <QObject>
#include <QSharedDataPointer>

class LaserItemPrivate;

class LaserItem : public QObject
{
    Q_OBJECT
public:
    explicit LaserItem(QObject* parent = nullptr);
    LaserItem(LaserItem* item, QObject* parent = nullptr);
    LaserItem(const LaserItem& item, QObject* parent = nullptr);
    virtual ~LaserItem();

private:
    QSharedDataPointer<LaserItemPrivate> d_ptr;
    friend class LaserItemPrivate;
};

#endif // LASERITEM_H