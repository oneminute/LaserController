#include "LaserLayer.h"

#include <QSharedData>

class LaserLayerPrivate : public QSharedData
{
public:
    LaserLayerPrivate()
        : m_id("")
        , m_type(LaserLayer::LLT_ENGRAVING)
        , m_minSpeed(0)
        , m_runSpeed(0)
        , m_laserPower(0)
        , m_engravingForward(false)
        , m_engravingStyle(0)
        , m_lineSpacing(0)
        , m_columnSpacing(0)
        , m_startX(0)
        , m_startY(0)
        , m_errorX(0)
        , m_errorY(0)
        , m_moveSpeed(0)
        , m_minSpeedPower(0)
        , m_runSpeedPower(0)
    {}
    QString m_id;
    LaserLayer::LayerType m_type;

    int m_minSpeed;
    int m_runSpeed;
    int m_laserPower;

    // engraving fields
    bool m_engravingForward;
    int m_engravingStyle;
    int m_lineSpacing;
    int m_columnSpacing;
    int m_startX;
    int m_startY;
    int m_errorX;
    int m_errorY;

    // cutting fields
    int m_moveSpeed;
    int m_minSpeedPower;
    int m_runSpeedPower;

    QList<LaserItem*> m_items;

    friend class LaserLayer;
};

LaserLayer::LaserLayer(const QString& id, LayerType type)
    : d_ptr(new LaserLayerPrivate)
{
    d_ptr->m_id = id;
    d_ptr->m_type = type;
}

LaserLayer::LaserLayer(const LaserLayer & other)
    : d_ptr(other.d_ptr)
{
}

LaserLayer::~LaserLayer()
{
}

LaserLayer & LaserLayer::operator=(const LaserLayer & other)
{
    d_ptr = other.d_ptr;
    return *this;
}

QString LaserLayer::id() const { return d_ptr->m_id; }

void LaserLayer::setId(const QString & id)
{
    d_ptr->m_id = id;
}

LaserLayer::LayerType LaserLayer::type() const { return d_ptr->m_type; }

int LaserLayer::minSpeed() const { return d_ptr->m_minSpeed; }

void LaserLayer::setMinSpeed(int minSpeed) { d_ptr->m_minSpeed = minSpeed; }

int LaserLayer::runSpeed() const { return d_ptr->m_runSpeed; }

void LaserLayer::setRunSpeed(int runSpeed) { d_ptr->m_runSpeed = runSpeed; }

int LaserLayer::laserPower() const { return d_ptr->m_laserPower; }

void LaserLayer::setLaserPower(int laserPower) { d_ptr->m_laserPower = laserPower; }

bool LaserLayer::engravingForward() const { return d_ptr->m_engravingForward; }

void LaserLayer::setEngravingForward(bool engravingForward) { d_ptr->m_engravingForward = engravingForward; }

int LaserLayer::engravingStyle() const { return d_ptr->m_engravingStyle; }

void LaserLayer::setEngravingStyle(int engravingStyle) { d_ptr->m_engravingStyle = engravingStyle; }

int LaserLayer::lineSpacing() const { return d_ptr->m_lineSpacing; }

void LaserLayer::setLineSpacing(int lineSpacing) { d_ptr->m_lineSpacing = lineSpacing; }

int LaserLayer::columnSpacing() const { return d_ptr->m_columnSpacing; }

void LaserLayer::setColumnSpacing(int columnSpacing) { d_ptr->m_columnSpacing = columnSpacing; }

QPoint LaserLayer::startPos() const { return QPoint(d_ptr->m_startX, d_ptr->m_startY); }

void LaserLayer::setStartPos(const QPoint & startPos) 
{ 
    d_ptr->m_startX = startPos.x(); 
    d_ptr->m_startY = startPos.y();
}

int LaserLayer::startX() const
{
    return d_ptr->m_startX;
}

void LaserLayer::setStartX(int x)
{
    d_ptr->m_startX = x;
}

int LaserLayer::startY() const
{
    return d_ptr->m_startY;
}

void LaserLayer::setStartY(int y)
{
    d_ptr->m_startY = y;
}

int LaserLayer::errorX() const { return d_ptr->m_errorX; }

void LaserLayer::setErrorX(int errorX) { d_ptr->m_errorX = errorX; }

int LaserLayer::errorY() const
{
    return d_ptr->m_errorY;
}

void LaserLayer::setErrorY(int errorY)
{
    d_ptr->m_errorY = errorY;
}

int LaserLayer::moveSpeed() const { return d_ptr->m_moveSpeed; }

void LaserLayer::setMoveSpeed(int moveSpeed) { d_ptr->m_moveSpeed = moveSpeed; }

int LaserLayer::minSpeedPower() const { return d_ptr->m_minSpeedPower; }

void LaserLayer::setMinSpeedPower(int minSpeedPower) { d_ptr->m_minSpeedPower = minSpeedPower; }

int LaserLayer::runSpeedPower() const { return d_ptr->m_runSpeedPower; }

void LaserLayer::setRunSpeedPower(int runSpeedPower) { d_ptr->m_runSpeedPower = runSpeedPower; }

