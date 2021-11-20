#ifndef SVGIMPORTER_H
#define SVGIMPORTER_H

#include <QObject>

#include "Importer.h"

class LaserPrimitive;

class SvgImporter : public Importer
{
    Q_OBJECT
public:
    explicit SvgImporter(QObject* parent = nullptr);
    virtual ~SvgImporter();

protected:
    virtual void importImpl(const QString& filename, LaserScene* scene, QList<LaserPrimitive*>& unavailables, ProgressItem* parentProgress, const QVariantMap& params = QVariantMap());

private:
    friend class Importer;
};

#endif // SVGIMPORTER_H
