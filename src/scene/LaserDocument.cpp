#include "LaserDocument.h"

#include <QSharedData>
#include <QList>

#include "LaserItem.h"
#include "PageInformation.h"

class LaserDocumentPrivate: public QSharedData
{
public:
    LaserDocumentPrivate()
    {}

private:
    QList<LaserItem*> items;
    PageInformation pageInfo;

    friend class LaserDocument;
};

LaserDocument::LaserDocument(QObject* parent)
    : QObject(parent)
    , d_ptr(new LaserDocumentPrivate)
{

}

LaserDocument::LaserDocument(const LaserDocument& other, QObject* parent)
    : QObject(parent)
    , d_ptr(other.d_ptr)
{

}

LaserDocument::~LaserDocument()
{
}

void LaserDocument::addItem(const LaserItem & item)
{
    //d_ptr->items.append(item);
}

void LaserDocument::addItem(LaserItem * item)
{
    d_ptr->items.append(item);
}

PageInformation LaserDocument::pageInformation() const
{
    return d_ptr->pageInfo;
}

