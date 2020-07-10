#ifndef PAGEINFORMATION_H
#define PAGEINFORMATION_H

#include <QObject>
#include <QSharedDataPointer>

class PageInformationPrivate;

class PageInformation : public QObject
{
    Q_OBJECT
public:
    enum Orientation
    {
        PORTRAIT,
        LANDSCAPE
    };
    explicit PageInformation(QObject* parent = nullptr);
    PageInformation(const PageInformation& other, QObject* parent = nullptr);
    virtual ~PageInformation();

    qreal width() const;
    void setWidth(qreal _width);

    qreal height() const;
    void setHeight(qreal _height);

    Orientation orientation() const;
    void setSetOrientation(Orientation orientation);

private:
    QSharedDataPointer<PageInformationPrivate> d_ptr;
    friend class PageInformationPrivate;
};

#endif // PAGEINFORMATION_H