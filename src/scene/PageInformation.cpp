#include "PageInformation.h"

#include <QSharedData>

class PageInformationPrivate : public QSharedData
{
public:
    PageInformationPrivate()
        : width(0)
        , height(0)
        , orientation(PageInformation::PORTRAIT)
    {

    }
private:
    qreal width;
    qreal height;
    PageInformation::Orientation orientation;
    friend class PageInformation;
};

PageInformation::PageInformation(QObject* parent)
    : QObject(parent)
    , d_ptr(new PageInformationPrivate)
{

}

PageInformation::PageInformation(const PageInformation& other, QObject* parent)
    : QObject(parent)
    , d_ptr(other.d_ptr)
{

}

PageInformation & PageInformation::operator=(const PageInformation & other)
{
    d_ptr = other.d_ptr;
    return *this;
}

PageInformation::~PageInformation()
{
}

qreal PageInformation::width() const
{
    return d_ptr->width;
}

void PageInformation::setWidth(qreal _width)
{
    d_ptr->width = _width;
}

qreal PageInformation::height() const
{
    return d_ptr->height;
}

void PageInformation::setHeight(qreal _height)
{
    d_ptr->height = _height;
}

PageInformation::Orientation PageInformation::orientation() const
{
    return d_ptr->orientation;
}

void PageInformation::setSetOrientation(Orientation orientation)
{
    d_ptr->orientation = orientation;
}
