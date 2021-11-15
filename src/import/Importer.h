#ifndef IMPORTER_H
#define IMPORTER_H

#include <QObject>
#include <QSharedPointer>
#include <QMap>
#include <QVariant>

class LaserDocument;
class LaserScene;
class ProgressItem;

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

    void import(const QString& filename, LaserScene* scene, ProgressItem* parentProgress, const QVariantMap& params = QVariantMap());
    
    static QSharedPointer<Importer> getImporter(QWidget* parentWnd, Types type);

    static QSharedPointer<Importer> getImporter(QWidget* parentWnd, const QString& type);

signals:
    void imported();

protected:
    virtual void importImpl(const QString& filename, LaserScene* scene, ProgressItem* parentProgress, const QVariantMap& params = QVariantMap()) = 0;
};

#endif // IMPORTER_H