#ifndef IMPORTER_H
#define IMPORTER_H

#include <QObject>
#include <QSharedPointer>

class LaserDocument;

class Importer : public QObject
{
    Q_OBJECT
public:
    enum Types
    {
        SVG = 0,
        CORELDRAW
    };

    Importer(QObject* parent = nullptr);

    virtual LaserDocument* import(const QString& filename = "") = 0;
    
    static QSharedPointer<Importer> getImporter(Types type);

signals:
    void imported();
};

#endif // IMPORTER_H