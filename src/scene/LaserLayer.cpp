#include "LaserLayer.h"

#include <QMessageBox>

#include "util/Utils.h"
#include "LaserDocument.h"
#include "LaserPrimitive.h"
#include "LaserScene.h"
#include "widget/LayerButton.h"

LaserLayer::LaserLayer(const QString& name, LaserLayerType type, LaserDocument* document)
    : QObject(document)
    , m_removable(true)
    , m_type(type)
    , m_minSpeed(60)
    , m_runSpeed(300)
    , m_laserPower(115)
    , m_engravingForward(true)
    , m_engravingStyle(0)
    , m_lineSpacing(7)
    , m_columnSpacing(0)
    , m_startX(25)
    , m_startY(0)
    , m_errorX(0)
    , m_minSpeedPower(60)
    , m_runSpeedPower(60)
    , m_doc(document)
    , m_lpi(30)
    , m_dpi(600)
    , m_button(nullptr)
    , m_exportable(true)
    , m_visible(true)
    , m_row(-1)
    , m_useHalftone(true)
{
    Q_ASSERT(document);
    setParent(document);
    setObjectName(utils::createUUID("layer_"));

    if (m_type == LLT_ENGRAVING)
    {
        m_minSpeed = 60;
        m_runSpeed = 300;
        m_laserPower = 115;
        m_minSpeedPower = 0;
        m_runSpeedPower = 900;
    }
    else if (m_type == LLT_CUTTING)
    {
        m_minSpeed = 15;
        m_runSpeed = 60;
        m_laserPower = 80;
        m_minSpeedPower = 700;
        m_runSpeedPower = 1000;
    }
}

LaserLayer::~LaserLayer()
{
    qDebug() << objectName();
}

QString LaserLayer::name() const 
{
    if (m_button)
    {
        return m_button->text();
    }
    else
    {
        return tr("undefined");
    }
}

LaserLayerType LaserLayer::type() const { return m_type; }

void LaserLayer::setType(LaserLayerType type)
{
    m_type = type;
}

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

bool LaserLayer::isEmpty() const
{
    return m_items.count() == 0;
}

QColor LaserLayer::color() const 
{
    if (m_button)
        return m_button->color();
    else
        return QColor();
}

LaserDocument * LaserLayer::document() const
{
    return m_doc;
}

void LaserLayer::bindButton(LayerButton * button)
{
    m_button = button;
    connect(button, &LayerButton::clicked, this, &LaserLayer::onClicked);
}

void LaserLayer::onClicked()
{
    LaserScene* scene = m_doc->scene();
    if (scene->selectedPrimitives().count() > 0)
    {
        for (LaserPrimitive* primitive : scene->selectedPrimitives())
        {
            scene->document()->addItem(primitive, this);
        }

        QList<LaserPrimitiveType> types;
        for (LaserPrimitive* primitive : m_items)
        {
            if (primitive->isShape())
            {
                if (!types.contains(LPT_SHAPE))
                {
                    types.append(LPT_SHAPE);
                }
            }
            else if (primitive->isBitmap())
            {
                if (!types.contains(LPT_BITMAP))
                {
                    types.append(LPT_BITMAP);
                }
            }
        }

        if (types.size() > 1)
        {
            setType(LLT_ENGRAVING);
        }
        else if (types[0] == LPT_SHAPE)
        {
            setType(LLT_CUTTING);
        }
        else if (types[0] == LPT_BITMAP)
        {
            setType(LLT_ENGRAVING);
        }
        scene->document()->updateLayersStructure();
    }
}

