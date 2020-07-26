#include "LaserLayer.h"

#include <QSharedData>

#include "LaserDocument.h"

LaserLayer::LaserLayer(LaserDocument* document)
    : QObject(document)
    , m_type(LLT_ENGRAVING)
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
}

LaserLayer::LaserLayer(const QString& id, LayerType type, LaserDocument* document)
    : QObject(document)
    , m_type(type)
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
    setObjectName(id);
}

LaserLayer::~LaserLayer()
{
    qDebug() << objectName();
}

QString LaserLayer::id() const 
{
    return objectName(); 
}

void LaserLayer::setId(const QString & id)
{
    setObjectName(id);
}

LayerType LaserLayer::type() const { return m_type; }

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

void LaserLayer::addItem(LaserItem * item)
{
    m_items.append(item);
    m_doc->updateLayersStructure();
}

QList<LaserItem*>& LaserLayer::items()
{
    return m_items;
}

LaserDocument * LaserLayer::document() const
{
    return m_doc;
}

