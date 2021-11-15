#ifndef SVGIMPORTER_H
#define SVGIMPORTER_H

#include <QObject>

#include "Importer.h"

class SvgImporter : public Importer
{
    Q_OBJECT
public:
    explicit SvgImporter(QObject* parent = nullptr);
    virtual ~SvgImporter();

protected:
    virtual void importImpl(const QString& filename, LaserScene* scene, ProgressItem* parentProgress, const QVariantMap& params = QVariantMap());

private:
    friend class Importer;
};

#endif // SVGIMPORTER_H
