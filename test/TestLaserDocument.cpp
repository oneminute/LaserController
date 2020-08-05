#include <TestLaserDocument.h>

void TestLaserDocument::initTestCase()
{
    m_doc = new LaserDocument();
}

void TestLaserDocument::documentTestCase()
{
    LaserRectItem* rectItem01 = new LaserRectItem(QRectF(10, 10, 30, 30), m_doc);
    m_doc->addItem(rectItem01);
}

void TestLaserDocument::cleanupTestCase()
{
    m_doc->destroy();
}