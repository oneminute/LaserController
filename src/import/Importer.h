#ifndef IMPORTER_H
#define IMPORTER_H

#include <QObject>
#include <QSharedPointer>

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

    Importer(QWidget* parentWnd, QObject* parent = nullptr);

    virtual LaserDocument* import(const QString& filename, LaserScene* scene) = 0;
    
    static QSharedPointer<Importer> getImporter(QWidget* parentWnd, Types type);

signals:
    void imported();

protected:
    QWidget* m_parentWnd;
};

#endif // IMPORTER_H