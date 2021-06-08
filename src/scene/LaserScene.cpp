#include "LaserScene.h"

#include "LaserDocument.h"
#include "LaserPrimitive.h"
#include "LaserLayer.h"

LaserScene::LaserScene(QObject* parent)
    : QGraphicsScene(parent)
    , m_doc(nullptr)
	, m_background(nullptr)
{

}

LaserScene::~LaserScene()
{
    clearSelection();
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
    m_doc = doc;

    m_doc->setParent(this);

    qDebug() << "page bounds in pixel:" << m_doc->pageBounds();
    m_background = addRect(m_doc->pageBounds(), QPen(Qt::gray, 0.2f, Qt::SolidLine), QBrush(Qt::white));
	setSceneRect(m_doc->pageBounds());
    QMap<QString, LaserPrimitive*> items = m_doc->primitives();
    for (QMap<QString, LaserPrimitive*>::iterator i = items.begin(); i != items.end(); i++)
    {
        this->addItem(i.value());
    }
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

void LaserScene::addLaserPrimitive(LaserPrimitive * primitive)
{
    m_doc->addPrimitive(primitive);
    addItem(primitive);
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

