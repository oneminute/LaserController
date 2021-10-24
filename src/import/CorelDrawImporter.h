#ifndef CORELDRAWIMPORTER_H
#define CORELDRAWIMPORTER_H

#include "common/common.h"
#include "Importer.h"

class ProgressItem;

class CorelDrawImporter : public Importer
{
    Q_OBJECT
public:
    explicit CorelDrawImporter(QObject* parent = nullptr);
    virtual ~CorelDrawImporter();

    virtual LaserDocument* import(const QString& filename, LaserScene* scene, ProgressItem* parentProgres, const QVariantMap& params = QVariantMap());
};

#endif // CORELDRAWIMPORTER_H