#include "LaserScene.h"

#include "LaserDocument.h"
#include "LaserItem.h"

LaserScene::LaserScene(QObject* parent)
    : QGraphicsScene(parent)
    , m_doc(nullptr)
{

}

LaserScene::~LaserScene()
{

}

void LaserScene::updateDocument(LaserDocument * doc)
{
    if (m_doc == doc)
    {
        clearDocument(false);
    }
    else
    {
        clearDocument(true);
    }

    QList<LaserItem*> items = doc->items();
    for (QList<LaserItem*>::iterator i = items.begin(); i != items.end(); i++)
    {
        //(*i)->bindGraphicsItem(this);
        this->addItem(*i);
    }
    m_doc = doc;
}

void LaserScene::clearDocument(bool delDoc)
{
    this->clear();

    if (delDoc && m_doc)
    {
        delete m_doc;
        m_doc = nullptr;
    }
}

//void LaserScene::draw(LaserViewer* view, QPaintEvent * event)
//{
//    if (!m_doc)
//        return;
//
//    QList<LaserItem*> items = m_doc->items();
//    /*for (QList<LaserItem*>::iterator i = items.begin(); i != items.end(); i++)
//    {
//        (*i)->draw(view, event);
//    }*/
//}
