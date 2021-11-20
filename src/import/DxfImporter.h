#ifndef DXFIMPORTER_H
#define DXFIMPORTER_H

#include "Importer.h"
#include "QStack"

class QTextStream;
class DxfNode;
class LaserPrimitive;

class DxfImporterPrivate;
class DxfImporter : public Importer
{
    Q_OBJECT
public:
    explicit DxfImporter(QObject* parent = nullptr);
    virtual ~DxfImporter();

protected:
    virtual void importImpl(const QString& filename, LaserScene* scene, QList<LaserPrimitive*>& unavailables, ProgressItem* parentProgress, const QVariantMap& params = QVariantMap());

private:
    QScopedPointer<DxfImporterPrivate> m_ptr;
    Q_DECLARE_PRIVATE_D(m_ptr, DxfImporter)
    Q_DISABLE_COPY(DxfImporter)

    friend class Importer;
};

#endif // DXFIMPORTER_H
