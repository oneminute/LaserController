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

    doc->setParent(this);
    QMap<QString, LaserPrimitive*> items = doc->items();
    for (QMap<QString, LaserPrimitive*>::iterator i = items.begin(); i != items.end(); i++)
    {
        this->addItem(i.value());
    }
    m_doc = doc;

    qDebug() << "page bounds:" << m_doc->pageBounds();
    m_background = addRect(m_doc->pageBounds(), QPen(Qt::gray, 0.2f, Qt::SolidLine));
}

void LaserScene::clearDocument(bool delDoc)
{
    this->clear();

    if (delDoc && m_doc)
    {
        m_doc->close();
        m_doc = nullptr;
    }
}

QList<LaserPrimitive*> LaserScene::selectedPrimitives() const
{
    QList<LaserPrimitive*> primitives;
    for (QGraphicsItem* item : selectedItems())
    {
        LaserPrimitive* primitive = dynamic_cast<LaserPrimitive*>(item);
        if (primitive)
        {
            primitives.append(primitive);
        }
    }
    return primitives;
}

