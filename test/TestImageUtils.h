#ifndef TESTIMAGEUTILS_H
#define TESTIMAGEUTILS_H

#include <QtTest>
#include <QList>

#include <scene/LaserDocument.h>
#include <scene/LaserLayer.h>
#include <scene/LaserPrimitive.h>

class TestImageUtils : public QObject
{
    Q_OBJECT
private slots:
    void generateDitchMatRecTestCase();
    void generateRoundSpiralMatTestCase();
    void generateRotatedPatternTestCase();

private:
};

#endif // TESTIMAGEUTILS_H
