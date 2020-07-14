#ifndef LASERDOCUMENT_H
#define LASERDOCUMENT_H

#include <QObject>
#include <QSharedPointer>
#include <QSharedDataPointer>

#include "PageInformation.h"

class LaserDocumentPrivate;
class LaserItem;

class LaserDocument : public QObject
{
    Q_OBJECT
public:
    explicit LaserDocument(QObject* parent = nullptr);
    LaserDocument(const LaserDocument& other, QObject* parent = nullptr);
    ~LaserDocument();

    void addItem(LaserItem* item);

    PageInformation pageInformation() const;
    QList<LaserItem*> items() const;

    qreal scale() const;
    void setScale(qreal scale);

private:
    QSharedDataPointer<LaserDocumentPrivate> d_ptr;
    friend class LaserDocumentPrivate;
};

#endif // LASERDOCUMENT_H