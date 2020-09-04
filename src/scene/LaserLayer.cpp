#include "LaserLayer.h"

#include <QSharedData>

#include "util/Utils.h"
#include "LaserDocument.h"
#include "LaserItem.h"

LaserLayer::LaserLayer(const QString& name, LaserLayerType type, LaserDocument* document)
    : QObject(document)
    , m_removable(true)
    , m_type(type)
    , m_name(name)
    , m_minSpeed(60)
    , m_runSpeed(300)
    , m_laserPower(60)
    , m_engravingForward(true)
    , m_engravingStyle(0)
    , m_lineSpacing(10)
    , m_columnSpacing(10)
    , m_startX(10)
    , m_startY(10)
    , m_errorX(0)
    , m_errorY(0)
    , m_moveSpeed(0)
    , m_minSpeedPower(60)
    , m_runSpeedPower(60)
    , m_doc(document)
{
    Q_ASSERT(document);
    setParent(document);
    setObjectName(utils::createUUID("layer_"));
}

LaserLayer::~LaserLayer()
{
    qDebug() << objectName();
}

QString LaserLayer::name() const 
{
    return m_name; 
}

void LaserLayer::setName(const QString & name)
{
    m_name = name;
}

LaserLayerType LaserLayer::type() const { return m_type; }

int LaserLayer::minSpeed() const { return m_minSpeed; }

void LaserLayer::setMinSpeed(int minSpeed) { m_minSpeed = minSpeed; }

int LaserLayer::runSpeed() const { return m_runSpeed; }

void LaserLayer::setRunSpeed(int runSpeed) { m_runSpeed = runSpeed; }

int LaserLayer::laserPower() const { return m_laserPower; }

void LaserLayer::setLaserPower(int laserPower) { m_laserPower = laserPower; }

bool LaserLayer::engravingForward() const { return m_engravingForward; }

void LaserLayer::setEngravingForward(bool engravingForward) { m_engravingForward = engravingForward; }

int LaserLayer::engravingStyle() const { return m_engravingStyle; }

void LaserLayer::setEngravingStyle(int engravingStyle) { m_engravingStyle = engravingStyle; }

int LaserLayer::lineSpacing() const { return m_lineSpacing; }

void LaserLayer::setLineSpacing(int lineSpacing) { m_lineSpacing = lineSpacing; }

int LaserLayer::columnSpacing() const { return m_columnSpacing; }

void LaserLayer::setColumnSpacing(int columnSpacing) { m_columnSpacing = columnSpacing; }

QPoint LaserLayer::startPos() const { return QPoint(m_startX, m_startY); }

void LaserLayer::setStartPos(const QPoint & startPos) 
{ 
    m_startX = startPos.x(); 
    m_startY = startPos.y();
}

int LaserLayer::startX() const
{
    return m_startX;
}

void LaserLayer::setStartX(int x)
{
    m_startX = x;
}

int LaserLayer::startY() const
{
    return m_startY;
}

void LaserLayer::setStartY(int y)
{
    m_startY = y;
}

int LaserLayer::errorX() const { return m_errorX; }

void LaserLayer::setErrorX(int errorX) { m_errorX = errorX; }

int LaserLayer::errorY() const
{
    return m_errorY;
}

void LaserLayer::setErrorY(int errorY)
{
    m_errorY = errorY;
}

int LaserLayer::moveSpeed() const { return m_moveSpeed; }

void LaserLayer::setMoveSpeed(int moveSpeed) { m_moveSpeed = moveSpeed; }

int LaserLayer::minSpeedPower() const { return m_minSpeedPower; }

void LaserLayer::setMinSpeedPower(int minSpeedPower) { m_minSpeedPower = minSpeedPower; }

int LaserLayer::runSpeedPower() const { return m_runSpeedPower; }

void LaserLayer::setRunSpeedPower(int runSpeedPower) { m_runSpeedPower = runSpeedPower; }

void LaserLayer::addItem(LaserPrimitive * item)
{
    if (m_items.contains(item))
        return;

    item->setLayer(this);
    m_items.append(item);
    m_doc->updateLayersStructure();
}

QList<LaserPrimitive*>& LaserLayer::items()
{
    return m_items;
}

void LaserLayer::removeItem(LaserPrimitive * item)
{
    if (!m_items.contains(item))
        return;

    item->setLayer(nullptr);
    m_items.removeOne(item);
    m_doc->updateLayersStructure();
}

LaserDocument * LaserLayer::document() const
{
    return m_doc;
}

