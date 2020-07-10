#include "LaserItem.h"

#include <QSharedData>

class LaserItemPrivate : public QSharedData
{
public:
    LaserItemPrivate()
    {}

private:

};

LaserItem::LaserItem(QObject* parent)
    : QObject(parent)
    , d_ptr(new LaserItemPrivate)
{

}

LaserItem::LaserItem(LaserItem* item, QObject* parent)
    : QObject(parent)
{
    if (item != nullptr)
    {
        d_ptr = item->d_ptr;
    }
}

LaserItem::LaserItem(const LaserItem& item, QObject* parent)
    : QObject(parent)
    , d_ptr(item.d_ptr)
{

}

LaserItem::~LaserItem()
{
}
