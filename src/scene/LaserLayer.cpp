#include "LaserLayer.h"

#include <QMessageBox>
#include <QJsonArray>

#include "util/Utils.h"
#include "LaserDocument.h"
#include "LaserPrimitive.h"
#include "LaserNodePrivate.h"
#include "LaserScene.h"
#include "widget/LayerButton.h"
#include "common/Config.h"

class LaserLayerPrivate: public LaserNodePrivate
{
    Q_DECLARE_PUBLIC(LaserLayer)
public:
    LaserLayerPrivate(LaserLayer* ptr)
        : LaserNodePrivate(ptr)
        , removable(true)
        , minSpeed(60)
        , runSpeed(300)
        , laserPower(115)
        , engravingForward(true)
        , engravingStyle(0)
        , lineSpacing(7)
        , columnSpacing(0)
        , startX(25)
        , startY(0)
        , errorX(0)
        , minSpeedPower(60)
        , runSpeedPower(60)
        , doc(nullptr)
        , lpi(60)
        , dpi(600)
        , button(nullptr)
        , exportable(true)
        , visible(true)
        , row(-1)
        , useHalftone(true)
        , isDefault(false)   
    {}

    bool removable;
    LaserLayerType type;

    // normal fields
    int minSpeed;
    int runSpeed;
    int laserPower;
    int minSpeedPower;
    int runSpeedPower;

    // engraving fields
    bool engravingForward;
    int engravingStyle;
    int lineSpacing;
    int columnSpacing;
    int startX;
    int startY;
    int errorX;

    // bitmap fields
    int lpi;
    int dpi;
    int row;
    bool useHalftone;

    LaserDocument* doc;
    LayerButton* button;

    bool exportable;
    bool visible;
	bool isDefault;

    QList<LaserPrimitive*> primitives;   
};

LaserLayer::LaserLayer(const QString& name, LaserLayerType type, LaserDocument* document, bool isDefault)
    : LaserNode(new LaserLayerPrivate(this), LNT_LAYER)
{
    Q_D(LaserLayer);
    Q_ASSERT(document);
    d->doc = document;
    d->type = type;
    d->isDefault = isDefault;
    d->nodeName = utils::createUUID("layer_");
    setParent(document);

    if (type == LLT_ENGRAVING)
    {
        d->minSpeed = Config::EngravingLayer::minSpeed();
        d->runSpeed = Config::EngravingLayer::runSpeed();
        d->laserPower = Config::EngravingLayer::laserPower();
        d->minSpeedPower = Config::EngravingLayer::minPowerRate();
        d->runSpeedPower = Config::EngravingLayer::maxPowerRate();
    }
    else if (type == LLT_CUTTING)
    {
        d->minSpeed = Config::CuttingLayer::minSpeed();
        d->runSpeed = Config::CuttingLayer::runSpeed();
        d->laserPower = Config::CuttingLayer::laserPower();
        d->minSpeedPower = Config::CuttingLayer::minPowerRate();
        d->runSpeedPower = Config::CuttingLayer::maxPowerRate();
    }
	d->useHalftone = Config::EngravingLayer::useHalftone();
	d->lpi = Config::EngravingLayer::LPI();
	d->dpi = Config::EngravingLayer::DPI();
}

LaserLayer::~LaserLayer()
{
    qLogD << "layer " << nodeName() << " destroyed.";
}

bool LaserLayer::removable() const 
{ 
    Q_D(const LaserLayer);
    return d->removable; 
}

void LaserLayer::setRemovable(bool removable) 
{ 
    Q_D(LaserLayer);
    d->removable = removable; 
}

QString LaserLayer::id() const 
{
    Q_D(const LaserLayer);
    return d->nodeName; 
}

QString LaserLayer::name() const 
{
    Q_D(const LaserLayer);
    if (d->button)
    {
        return d->button->text();
    }
    else
    {
        return tr("undefined");
    }
}

LaserLayerType LaserLayer::type() const 
{
    Q_D(const LaserLayer);
    return d->type; 
}

void LaserLayer::setType(LaserLayerType type)
{
    Q_D(LaserLayer);
    d->type = type;
}

int LaserLayer::minSpeed() const 
{
    Q_D(const LaserLayer);
    return d->minSpeed; 
}

void LaserLayer::setMinSpeed(int minSpeed) 
{
    Q_D(LaserLayer);
    d->minSpeed = minSpeed; 
}

int LaserLayer::runSpeed() const 
{
    Q_D(const LaserLayer);
    return d->runSpeed; 
}

void LaserLayer::setRunSpeed(int runSpeed) 
{
    Q_D(LaserLayer);
    d->runSpeed = runSpeed; 
}

int LaserLayer::laserPower() const 
{
    Q_D(const LaserLayer);
    return d->laserPower; 
}

void LaserLayer::setLaserPower(int laserPower) 
{
    Q_D(LaserLayer);
    d->laserPower = laserPower; 
}

bool LaserLayer::engravingForward() const 
{
    Q_D(const LaserLayer);
    return d->engravingForward; 
}

void LaserLayer::setEngravingForward(bool engravingForward) 
{ 
    Q_D(LaserLayer);
    d->engravingForward = engravingForward; 
}

int LaserLayer::engravingStyle() const 
{ 
    Q_D(const LaserLayer);
    return d->engravingStyle; 
}

void LaserLayer::setEngravingStyle(int engravingStyle) 
{ 
    Q_D(LaserLayer);
    d->engravingStyle = engravingStyle; 
}

int LaserLayer::lineSpacing() const 
{ 
    Q_D(const LaserLayer);
    return d->lineSpacing; 
}

void LaserLayer::setLineSpacing(int lineSpacing) 
{ 
    Q_D(LaserLayer);
    d->lineSpacing = lineSpacing; 
}

int LaserLayer::columnSpacing() const 
{ 
    Q_D(const LaserLayer);
    return d->columnSpacing; 
}

void LaserLayer::setColumnSpacing(int columnSpacing) 
{ 
    Q_D(LaserLayer);
    d->columnSpacing = columnSpacing; 
}

QPoint LaserLayer::startPos() const 
{ 
    Q_D(const LaserLayer);
    return QPoint(d->startX, d->startY); 
}

void LaserLayer::setStartPos(const QPoint & startPos) 
{ 
    Q_D(LaserLayer);
    d->startX = startPos.x(); 
    d->startY = startPos.y();
}

int LaserLayer::startX() const
{
    Q_D(const LaserLayer);
    return d->startX;
}

void LaserLayer::setStartX(int x)
{
    Q_D(LaserLayer);
    d->startX = x;
}

int LaserLayer::startY() const
{
    Q_D(const LaserLayer);
    return d->startY;
}

void LaserLayer::setStartY(int y)
{
    Q_D(LaserLayer);
    d->startY = y;
}

int LaserLayer::errorX() const 
{ 
    Q_D(const LaserLayer);
    return d->errorX; 
}

void LaserLayer::setErrorX(int errorX) 
{ 
    Q_D(LaserLayer);
    d->errorX = errorX; 
}

int LaserLayer::minSpeedPower() const 
{ 
    Q_D(const LaserLayer);
    return d->minSpeedPower; 
}

void LaserLayer::setMinSpeedPower(int minSpeedPower) 
{ 
    Q_D(LaserLayer);
    d->minSpeedPower = minSpeedPower; 
}

int LaserLayer::runSpeedPower() const 
{ 
    Q_D(const LaserLayer);
    return d->runSpeedPower; 
}

void LaserLayer::setRunSpeedPower(int runSpeedPower) 
{ 
    Q_D(LaserLayer);
    d->runSpeedPower = runSpeedPower; 
}

void LaserLayer::addPrimitive(LaserPrimitive * item)
{
    Q_D(LaserLayer);
    if (d->primitives.contains(item))
        return;

    item->setLayer(this);
    d->primitives.append(item);
    d->childNodes.append(item);
    d->doc->updateLayersStructure();
}

QList<LaserPrimitive*>& LaserLayer::primitives()
{
    Q_D(LaserLayer);
    return d->primitives;
}

void LaserLayer::removePrimitive(LaserPrimitive * item)
{
    Q_D(LaserLayer);
    if (!d->primitives.contains(item))
        return;

    //item->setLayer(nullptr);
    d->primitives.removeOne(item);
    d->doc->updateLayersStructure();
}

bool LaserLayer::isEmpty() const
{
    Q_D(const LaserLayer);
    return d->primitives.count() == 0;
}

QColor LaserLayer::color() const 
{
    Q_D(const LaserLayer);
    if (d->button)
        return d->button->color();
    else
        return QColor();
}

int LaserLayer::lpi() const 
{ 
    Q_D(const LaserLayer);
    return d->lpi; 
}

void LaserLayer::setLpi(int lpi) 
{ 
    Q_D(LaserLayer);
    d->lpi = lpi; 
}

int LaserLayer::dpi() const 
{ 
    Q_D(const LaserLayer);
    return d->dpi; 
}

void LaserLayer::setDpi(int dpi) 
{ 
    Q_D(LaserLayer);
    d->dpi = dpi; 
}

LaserDocument * LaserLayer::document() const
{
    Q_D(const LaserLayer);
    return d->doc;
}

void LaserLayer::bindButton(LayerButton * button, int index)
{
    Q_D(LaserLayer);
    d->button = button;
	d->button->setLayerIndex(index);
	d->button->setEnabled(true);
    connect(button, &LayerButton::clicked, this, &LaserLayer::onClicked);
    d->nodeName = button->text();
	m_index = index;
}

bool LaserLayer::exportable() const 
{ 
    Q_D(const LaserLayer);
    return d->exportable; 
}

void LaserLayer::setExportable(bool value) 
{ 
    Q_D(LaserLayer);
    d->exportable = value; 
}

bool LaserLayer::visible() const 
{ 
    Q_D(const LaserLayer);
    return d->visible; 
}

void LaserLayer::setVisible(bool visible) 
{ 
    Q_D(LaserLayer);
    d->visible = visible; 
}

int LaserLayer::row() const 
{ 
    Q_D(const LaserLayer);
    return d->row; 
}

void LaserLayer::setRow(int row) 
{ 
    Q_D(LaserLayer);
    d->row = row; 
}

bool LaserLayer::useHalftone() const 
{ 
    Q_D(const LaserLayer);
    return d->useHalftone; 
}

void LaserLayer::setUseHalftone(bool value) 
{ 
    Q_D(LaserLayer);
    d->useHalftone = value; 
}

bool LaserLayer::isDefault() const 
{ 
    Q_D(const LaserLayer);
    return d->isDefault; 
}

bool LaserLayer::isAvailable() const
{
    Q_D(const LaserLayer);
    return !d->primitives.isEmpty();
}

QJsonObject LaserLayer::toJson(QWidget* window)
{
	QJsonObject object;
	QList<LaserPrimitive*> primitives = this->primitives();
	if (primitives.size() <= 0) {
		return object;
	}
	QJsonArray array;
	object.insert("name", this->name());
	object.insert("type", this->type());
	
	for (int i = 0; i < primitives.size(); i++) {
		LaserPrimitive* primitive = primitives[i];
		QString className = primitive->metaObject()->className();
		if (className == "LaserEllipse") {
			LaserEllipse* ellipse = qobject_cast<LaserEllipse*>(primitive);
			//array.insert(i, ellipse->toJson());
			array.append(ellipse->toJson());
		}
		else if (className == "LaserLine") {
			LaserLine* line = qobject_cast<LaserLine*>(primitive);
			array.append(line->toJson());
		}
		else if (className == "LaserRect") {
			LaserRect* rect = qobject_cast<LaserRect*>(primitive);
			array.append(rect->toJson());
		}
		else if (className == "LaserPolyline") {
			LaserPolyline* polyline = qobject_cast<LaserPolyline*>(primitive);
			array.append(polyline->toJson());
		}
		else if (className == "LaserPolygon") {
			LaserPolygon* polygon = qobject_cast<LaserPolygon*>(primitive);
			array.append(polygon->toJson());
		}
		else if (className == "LaserBitmap") {
			LaserBitmap* bitmap = qobject_cast<LaserBitmap*>(primitive);
			array.append(bitmap->toJson());
		}
        else if (className == "LaserText") {
            LaserText* text = qobject_cast<LaserText*>(primitive);
            array.append(text->toJson());
        }
		else {
			QMessageBox::critical(window, "critical", "can't save, the primitive class don't exit.");
		}
		
	}
	object.insert("primitives", array);
	return object;
}

int LaserLayer::index()
{
	return m_index;
}

void LaserLayer::setIndex(int i)
{
	m_index = i;
}

void LaserLayer::onClicked()
{
    Q_D(LaserLayer);
    LaserScene* scene = d->doc->scene();
    if (scene->selectedPrimitives().count() > 0)
    {
        for (LaserPrimitive* primitive : scene->selectedPrimitives())
        {
            scene->document()->addPrimitive(primitive, this);
        }

        QList<LaserPrimitiveType> types;
        for (LaserPrimitive* primitive : d->primitives)
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

