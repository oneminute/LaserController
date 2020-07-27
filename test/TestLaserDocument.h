#ifndef TESTLASERDOCUMENT_H
#define TESTLASERDOCUMENT_H

#include <QtTest>
#include <QList>

#include <scene/LaserDocument.h>
#include <scene/LaserLayer.h>
#include <scene/LaserItem.h>

class TestLaserDocument : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void documentTestCase();
    void cleanupTestCase();

private:
    LaserDocument* m_doc;
    QList<LaserLayer*> m_layers;
    QList<LaserPrimitive*> m_items;
};

#endif // TESTLASERDOCUMENT_H