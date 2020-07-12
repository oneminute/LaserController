#include "LaserItem.h"

#include <QSharedData>

//class LaserItemPrivate : public QSharedData
//{
//public:
//    LaserItemPrivate()
//    {}
//
//private:
//
//};
//
LaserItem::LaserItem(LaserDocument* doc)
    : m_graphicsItem(nullptr)
    , m_doc(doc)
    //, d_ptr(new LaserItemPrivate)
{

}

//LaserItem::LaserItem(LaserItem* item, QObject* parent)
//    : QObject(parent)
//{
//    if (item != nullptr)
//    {
//        d_ptr = item->d_ptr;
//    }
//}
//
//LaserItem::LaserItem(const LaserItem& item, QObject* parent)
//    : QObject(parent)
//    , d_ptr(item.d_ptr)
//{
//
//}

LaserItem::~LaserItem()
{
    if (m_graphicsItem)
    {
        delete m_graphicsItem;
        m_graphicsItem = nullptr;
    }
}

//LaserItem & LaserItem::operator=(const LaserItem & other)
//{
//    this->d_ptr = other.d_ptr;
//}

LaserArcItem::LaserArcItem(const QPainterPath & path, LaserDocument* doc)
    : LaserShapeItem(doc)
    , m_path(path)
{
}

LaserArcItem::~LaserArcItem()
{
}

LaserShapeItem::LaserShapeItem(LaserDocument* doc)
    : LaserItem(doc)
{
}
