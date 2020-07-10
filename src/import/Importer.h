#ifndef IMPORTER_H
#define IMPORTER_H

#include <QObject>
#include <QSharedPointer>

#include "scene/LaserDocument.h"

class Importer : public QObject
{
    Q_OBJECT
public:
    enum Types
    {
        SVG = 0
    };

    Importer(QObject* parent = nullptr);

    virtual LaserDocument import(const QString& filename) = 0;
    
    static QSharedPointer<Importer> getImporter(Types type);

private:
};

#endif // IMPORTER_H