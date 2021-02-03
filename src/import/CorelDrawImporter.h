#ifndef CORELDRAWIMPORTER_H
#define CORELDRAWIMPORTER_H

#include "common/common.h"
#include "Importer.h"

class CorelDrawImporter : public Importer
{
    Q_OBJECT
public:
    explicit CorelDrawImporter(QObject* parent = nullptr);
    virtual ~CorelDrawImporter();

    virtual LaserDocument* import(const QString& filename, LaserScene* scene, const QVariantMap& params = QVariantMap());
};

#endif // CORELDRAWIMPORTER_H