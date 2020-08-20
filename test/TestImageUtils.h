#ifndef TESTIMAGEUTILS_H
#define TESTIMAGEUTILS_H

#include <QtTest>
#include <QList>

#include <scene/LaserDocument.h>
#include <scene/LaserLayer.h>
#include <scene/LaserItem.h>

class TestImageUtils : public QObject
{
    Q_OBJECT
private slots:
    void generateDitchMatRecTestCase();

private:
};

#endif // TESTIMAGEUTILS_H
