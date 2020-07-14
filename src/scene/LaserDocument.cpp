#include "LaserDocument.h"

#include <QSharedData>
#include <QList>

#include "LaserItem.h"
#include "PageInformation.h"

class LaserDocumentPrivate: public QSharedData
{
public:
    LaserDocumentPrivate()
        : scale(1.0)
    {}

    ~LaserDocumentPrivate()
    {
        qDeleteAll(items);
    }

private:
    QList<LaserItem*> items;
    PageInformation pageInfo;
    qreal scale;

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

void LaserDocument::addItem(LaserItem * item)
{
    d_ptr->items.append(item);
}

PageInformation LaserDocument::pageInformation() const
{
    return d_ptr->pageInfo;
}

QList<LaserItem*> LaserDocument::items() const
{
    return d_ptr->items;
}

qreal LaserDocument::scale() const
{
    return d_ptr->scale;
}

void LaserDocument::setScale(qreal scale)
{
    d_ptr->scale = scale;
}

