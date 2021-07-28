#ifndef IMPORTER_H
#define IMPORTER_H

#include <QObject>
#include <QSharedPointer>
#include <QMap>
#include <QVariant>

class LaserDocument;
class LaserScene;

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

    virtual LaserDocument* import(const QString& filename, LaserScene* scene, const QVariantMap& params = QVariantMap()) = 0;
    
    static QSharedPointer<Importer> getImporter(QWidget* parentWnd, Types type);

    static QSharedPointer<Importer> getImporter(QWidget* parentWnd, const QString& type);

signals:
    void imported();

protected:
};

#endif // IMPORTER_H