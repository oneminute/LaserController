#include "LaserLayer.h"

#include <QMessageBox>
#include <QJsonArray>
#include <QList>

#include "LaserApplication.h"
#include "LaserDocument.h"
#include "LaserScene.h"
#include "common/Config.h"
#include "laser/LaserDevice.h"
#include "primitive/LaserPrimitiveHeaders.h"
#include "ui/LaserControllerWindow.h"
#include "util/Utils.h"
#include "util/WidgetUtils.h"
#include "widget/LayerButton.h"

class LaserLayerPrivate: public ILaserDocumentItemPrivate
{
    Q_DECLARE_PUBLIC(LaserLayer)
public:
    LaserLayerPrivate(LaserLayer* ptr)
        : ILaserDocumentItemPrivate(ptr, LNT_LAYER)
        , removable(true)
        , type(LLT_CUTTING)
        , engravingForward(true)
        , engravingStyle(0)
        , startX(25)
        , startY(0)
        , errorX(0)
        , cuttingRunSpeed(Config::CuttingLayer::runSpeed())
        , cuttingMinSpeedPower(Config::CuttingLayer::minPower())
        , cuttingRunSpeedPower(Config::CuttingLayer::maxPower())
        , engravingRunSpeed(Config::EngravingLayer::runSpeed())
        , engravingLaserPower(Config::EngravingLayer::laserPower())
        , engravingMinSpeedPower(Config::EngravingLayer::minPower())
        , engravingRunSpeedPower(Config::EngravingLayer::maxPower())
        , engravingRowInterval(Config::EngravingLayer::rowInterval())
        , engravingEnableCutting(Config::EngravingLayer::enableCutting())
        , fillingRunSpeed(Config::FillingLayer::runSpeed())
        , fillingMinSpeedPower(Config::FillingLayer::minPower())
        , fillingRunSpeedPower(Config::FillingLayer::maxPower())
        , fillingRowInterval(Config::FillingLayer::rowInterval())
        , fillingEnableCutting(Config::FillingLayer::enableCutting())
        , fillingType(Config::FillingLayer::fillingType())
        , lpi(Config::EngravingLayer::LPI())
        , dpi(Config::EngravingLayer::DPI())
        , useHalftone(Config::EngravingLayer::useHalftone())
        , halftoneAngles(Config::EngravingLayer::halftoneAngles())
        , stampBoundingDistance(Config::StampLayer::boundingDistance())
        , exportable(true)
        , visible(true)
        , row(-1)
        //, halftoneGridSize(Config::EngravingLayer::halftoneGridSize())
        , doc(nullptr)
        , isDefault(false)  
    {}

    bool removable;
    LaserLayerType type;
    QString name;
    QColor color;

    // normal fields
    int cuttingRunSpeed;
    int cuttingMinSpeedPower;
    int cuttingRunSpeedPower;
    int engravingRunSpeed;
    int engravingLaserPower;
    int engravingMinSpeedPower;
    int engravingRunSpeedPower;
    int engravingRowInterval;
    bool engravingEnableCutting;
    int fillingRunSpeed;
    int fillingMinSpeedPower;
    int fillingRunSpeedPower;
    int fillingRowInterval;
    bool fillingEnableCutting;
    int fillingType;
    int stampBoundingDistance;

    // engraving fields
    bool engravingForward;
    int engravingStyle;
    int startX;
    int startY;
    int errorX;

    // bitmap fields
    int lpi;
    int dpi;
    int row;
    bool useHalftone;
    qreal halftoneAngles;
    //int halftoneGridSize;

    LaserDocument* doc;

    bool exportable;
    bool visible;
	bool isDefault;

    QList<LaserPrimitive*> primitives;
    QMap<QString, LaserPrimitive*> primitiveMap;
};

LaserLayer::LaserLayer(const QString& name, LaserLayerType type, LaserDocument* document, bool isDefault, QCheckBox* box)
    : ILaserDocumentItem(LNT_LAYER, new LaserLayerPrivate(this))
{
    Q_D(LaserLayer);
    Q_ASSERT(document);
    d->doc = document;
    d->type = type;
    d->isDefault = isDefault;
    d->name = document->newLayerName();
    setParent(document);

    m_checkBox = box;
}

LaserLayer::~LaserLayer()
{
}

void LaserLayer::init()
{
    Q_D(LaserLayer);
    d->cuttingRunSpeed = Config::CuttingLayer::runSpeed();
    d->cuttingMinSpeedPower = Config::CuttingLayer::minPower();
    d->cuttingRunSpeedPower = Config::CuttingLayer::maxPower();
    d->engravingRunSpeed = Config::EngravingLayer::runSpeed();
    d->engravingLaserPower = Config::EngravingLayer::laserPower();
    d->engravingMinSpeedPower = Config::EngravingLayer::minPower();
    d->engravingRunSpeedPower = Config::EngravingLayer::maxPower();
    d->engravingRowInterval = Config::EngravingLayer::rowInterval();
    d->engravingEnableCutting = Config::EngravingLayer::enableCutting();
    d->fillingRunSpeed = Config::FillingLayer::runSpeed();
    d->fillingMinSpeedPower = Config::FillingLayer::minPower();
    d->fillingRunSpeedPower = Config::FillingLayer::maxPower();
    d->fillingRowInterval = Config::FillingLayer::rowInterval();
    d->fillingEnableCutting = Config::FillingLayer::enableCutting();
    d->fillingType = Config::FillingLayer::fillingType();
    d->lpi = Config::EngravingLayer::LPI();
    d->dpi = Config::EngravingLayer::DPI();
    d->useHalftone = Config::EngravingLayer::useHalftone();
    d->halftoneAngles = Config::EngravingLayer::halftoneAngles();
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

QString LaserLayer::name() const 
{
    Q_D(const LaserLayer);
    return d->name;
}

void LaserLayer::setName(const QString& value)
{
    Q_D(LaserLayer);
    d->name = value;
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

int LaserLayer::cuttingRunSpeed() const
{
    Q_D(const LaserLayer);
    return d->cuttingRunSpeed;
}

void LaserLayer::setCuttingRunSpeed(int runSpeed)
{
    Q_D(LaserLayer);
    d->cuttingRunSpeed = runSpeed;
}

int LaserLayer::cuttingMinSpeedPower() const
{
    Q_D(const LaserLayer);
    return d->cuttingMinSpeedPower;
}

void LaserLayer::setCuttingMinSpeedPower(int minSpeedPower)
{
    Q_D(LaserLayer);
    d->cuttingMinSpeedPower = minSpeedPower;
}

int LaserLayer::cuttingRunSpeedPower() const
{
    Q_D(const LaserLayer);
    return d->cuttingRunSpeedPower;
}

void LaserLayer::setCuttingRunSpeedPower(int runSpeedPower)
{
    Q_D(LaserLayer);
    d->cuttingRunSpeedPower = runSpeedPower;
}

int LaserLayer::engravingRunSpeed() const
{
    Q_D(const LaserLayer);
    return d->engravingRunSpeed;
}

void LaserLayer::setEngravingRunSpeed(int runSpeed)
{
    Q_D(LaserLayer);
    d->engravingRunSpeed = runSpeed;
}

int LaserLayer::engravingLaserPower() const
{
    Q_D(const LaserLayer);
    return d->engravingLaserPower;
}

void LaserLayer::setEngravingLaserPower(int laserPower)
{
    Q_D(LaserLayer);
    d->engravingLaserPower = laserPower;
}

int LaserLayer::engravingMinSpeedPower() const
{
    Q_D(const LaserLayer);
    return d->engravingMinSpeedPower;
}

void LaserLayer::setEngravingMinSpeedPower(int minSpeedPower)
{
    Q_D(LaserLayer);
    d->engravingMinSpeedPower = minSpeedPower;
}

int LaserLayer::engravingRunSpeedPower() const
{
    Q_D(const LaserLayer);
    return d->engravingRunSpeedPower;
}

void LaserLayer::setEngravingRunSpeedPower(int runSpeedPower)
{
    Q_D(LaserLayer);
    d->engravingRunSpeedPower = runSpeedPower;
}

int LaserLayer::engravingRowInterval() const 
{ 
    Q_D(const LaserLayer);
    return d->engravingRowInterval; 
}

void LaserLayer::setEngravingRowInterval(int rowInterval) 
{ 
    Q_D(LaserLayer);
    d->engravingRowInterval = rowInterval; 
}

bool LaserLayer::engravingEnableCutting() const
{
    Q_D(const LaserLayer);
    return d->engravingEnableCutting;
}

void LaserLayer::setEngravingEnableCutting(bool cutting)
{
    Q_D(LaserLayer);
    d->engravingEnableCutting = cutting;
}

int LaserLayer::fillingRunSpeed() const
{
    Q_D(const LaserLayer);
    return d->fillingRunSpeed;
}

void LaserLayer::setFillingRunSpeed(int runSpeed)
{
    Q_D(LaserLayer);
    d->fillingRunSpeed = runSpeed;
}

int LaserLayer::fillingMinSpeedPower() const
{
    Q_D(const LaserLayer);
    return d->fillingMinSpeedPower;
}

void LaserLayer::setFillingMinSpeedPower(int minSpeedPower)
{
    Q_D(LaserLayer);
    d->fillingMinSpeedPower = minSpeedPower;
}

int LaserLayer::fillingRunSpeedPower() const
{
    Q_D(const LaserLayer);
    return d->fillingRunSpeedPower;
}

void LaserLayer::setFillingRunSpeedPower(int runSpeedPower)
{
    Q_D(LaserLayer);
    d->fillingRunSpeedPower = runSpeedPower;
}

int LaserLayer::fillingRowInterval() const 
{ 
    Q_D(const LaserLayer);
    return d->fillingRowInterval; 
}

void LaserLayer::setFillingRowInterval(int rowInterval) 
{ 
    Q_D(LaserLayer);
    d->fillingRowInterval = rowInterval; 
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

bool LaserLayer::fillingEnableCutting() const
{
    Q_D(const LaserLayer);
    return d->fillingEnableCutting;
}

void LaserLayer::setFillingEnableCutting(bool cutting)
{
    Q_D(LaserLayer);
    d->fillingEnableCutting = cutting;
}

int LaserLayer::fillingType() const
{
    Q_D(const LaserLayer);
    return d->fillingType;
}

void LaserLayer::setFillingType(int type)
{
    Q_D(LaserLayer);
    d->fillingType = type;
}

int LaserLayer::stampBoundingDistance() const
{
    Q_D(const LaserLayer);
    return d->stampBoundingDistance;
}

void LaserLayer::setStampBoundingDistance(int distance)
{
    Q_D(LaserLayer);
    d->stampBoundingDistance = distance;
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

void LaserLayer::addPrimitive(LaserPrimitive * item)
{
    Q_D(LaserLayer);
    if (item->layer()) {
        item->layer()->removePrimitive(item);
    }
    item->setLayer(this);
    d->primitives.append(item);
    d->primitiveMap.insert(item->id(), item);
    d->doc->updateLayersStructure();
}

const QList<LaserPrimitive*>& LaserLayer::primitives()
{
    Q_D(LaserLayer);
    return d->primitives;
}

void LaserLayer::removePrimitive(LaserPrimitive * item, bool itemKeepLayer)
{
    Q_D(LaserLayer);
    if (itemKeepLayer)
    {
        d->primitives.removeOne(item);
        d->primitiveMap.remove(item->id());
    }
    else {
        item->setLayer(nullptr);
    }
    d->doc->updateLayersStructure();
}

void LaserLayer::removePrimitiveById(const QString& id)
{
    Q_D(LaserLayer);
    if (d->primitiveMap.contains(id))
    {
        LaserPrimitive* primitive = d->primitiveMap[id];
        removePrimitive(primitive);
    }
}

bool LaserLayer::isEmpty() const
{
    Q_D(const LaserLayer);
    return d->primitives.isEmpty();
}

QColor LaserLayer::color() const 
{
    Q_D(const LaserLayer);
    return d->color;
}

void LaserLayer::setColor(const QColor& value)
{
    Q_D(LaserLayer);
    d->color = value;
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
    for (LaserPrimitive* primitive : primitives()) {
        primitive->setVisible(visible);
    }
    

    if (m_checkBox && m_checkBox->isChecked() != visible) {
        m_checkBox->setChecked(visible);
    }
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

qreal LaserLayer::halftoneAngles() const
{
    Q_D(const LaserLayer);
    return d->halftoneAngles;
}

void LaserLayer::setHalftoneAngles(qreal angles)
{
    Q_D(LaserLayer);
    d->halftoneAngles = angles;
}

//bool LaserLayer::isDefault() const
//{ 
//    Q_D(const LaserLayer);
//    return d->isDefault; 
//}

bool LaserLayer::isAvailable() const
{
    Q_D(const LaserLayer);
    return !d->primitives.isEmpty() && exportable() && visible();
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
    object.insert("exportable", this->exportable());
    object.insert("visible", this->visible());
    object.insert("cuttingRunSpeed", this->cuttingRunSpeed());
    object.insert("cuttingMinSpeedPower", this->cuttingMinSpeedPower());
    object.insert("cuttingRunSpeedPower", this->cuttingRunSpeedPower());
    object.insert("engravingRunSpeed", this->engravingRunSpeed());
    object.insert("engravingLaserPower", this->engravingLaserPower());
    object.insert("engravingMinSpeedPower", this->engravingMinSpeedPower());
    object.insert("engravingRunSpeedPower", this->engravingRunSpeedPower());
    object.insert("engravingRowInterval", this->engravingRowInterval());
    object.insert("engravingEnableCutting", this->engravingEnableCutting());
    object.insert("fillingRunSpeed", this->fillingRunSpeed());
    object.insert("fillingMinSpeedPower", this->fillingMinSpeedPower());
    object.insert("fillingRunSpeedPower", this->fillingRunSpeedPower());
    object.insert("fillingRowInterval", this->fillingRowInterval());
    object.insert("fillingEnableCutting", this->fillingEnableCutting());
    object.insert("fillingType", this->fillingType());
    object.insert("errorX", this->errorX());
    object.insert("useHalftone", this->useHalftone());
    object.insert("lpi", this->lpi());
    object.insert("dpi", this->dpi());
    object.insert("halftoneAngles", this->halftoneAngles());
    object.insert("stampBoundingDistance", this->stampBoundingDistance());
    //object.insert("halftoneGridSize", this->halftoneGridSize());
	
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
        else if (className == "LaserPath") {
            LaserPath* path = qobject_cast<LaserPath*>(primitive);
            array.append(path->toJson());
        }
        else if (className == "LaserStar") {
            LaserStar* star = qobject_cast<LaserStar*>(primitive);
            array.append(star->toJson());
        }
        else if (className == "LaserPartyEmblem") {
            LaserPartyEmblem* star = qobject_cast<LaserPartyEmblem*>(primitive);
            array.append(star->toJson());
        }
        else if (className == "LaserRing") {
            LaserRing* ring = qobject_cast<LaserRing*>(primitive);
            array.append(ring->toJson());
        }
        else if (className == "LaserFrame") {
            LaserFrame* frame = qobject_cast<LaserFrame*>(primitive);
            array.append(frame->toJson());
        }
        else if (className == "LaserHorizontalText") {
            LaserHorizontalText* hText = qobject_cast<LaserHorizontalText*>(primitive);
            array.append(hText->toJson());
        }
        else if (className == "LaserVerticalText") {
            LaserVerticalText* text = qobject_cast<LaserVerticalText*>(primitive);
            array.append(text->toJson());
        }
        else if (className == "LaserCircleText") {
            LaserCircleText* cText = qobject_cast<LaserCircleText*>(primitive);
            array.append(cText->toJson());
        }
        else if (className == "LaserStampBitmap") {
            LaserStampBitmap* stampBitmap = qobject_cast<LaserStampBitmap*>(primitive);
            array.append(stampBitmap->toJson());
        }
		else {
			QMessageBox::critical(window, "critical", "can't save, "+className+" can't to json.");
            break;
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

QRect LaserLayer::boundingRect() const
{
    Q_D(const LaserLayer);
    QRect bounding;
    utils::boundingRect(d->primitives, bounding);
    return bounding;
}

QPoint LaserLayer::position() const 
{
    return boundingRect().topLeft();
}

QCheckBox * LaserLayer::checkBox()
{
    return m_checkBox;
}

void LaserLayer::setCheckBox(QCheckBox * box)
{
    m_checkBox = box;
}

void LaserLayer::setSelected()
{
    Q_D(LaserLayer);
    LaserScene* scene = d->doc->scene();
    if (!scene->selectedPrimitives().isEmpty())
    {
        int shapes = 0;
        int bitmaps = 0;
        int stamps = 0;
        int texts = 0;
        for (LaserPrimitive* primitive : scene->selectedPrimitives())
        {
            //scene->addLaserPrimitive(primitive, this, false);
            if (primitive->isShape())
            {
                shapes++;
            }
            else if (primitive->isBitmap())
            {
                bitmaps++;
            }
            else if (primitive->isStamp())
            {
                stamps++;
            }
            else if (primitive->isText())
            {
                texts++;
            }
        }
        int types = 0;
        if (shapes > 0)
            types++;
        if (bitmaps > 0)
            types++;
        if (stamps > 0)
            types++;
        if (texts > 0)
            types++;

        if (types > 1)
        {
            widgetUtils::showWarningMessage(
                LaserApplication::mainWindow,
                tr("Warning"),
                tr("You have selected multiple types of primitives. Please select primitives with the same types."));
            return;
        }

        for (LaserPrimitive* primitive : scene->selectedPrimitives())
        {
            scene->document()->addPrimitive(primitive, this);
        }
        if (bitmaps > 0)
        {
            setType(LLT_ENGRAVING);
        }
        else if (shapes > 0)
        {
            setType(LLT_CUTTING);
        }
        else if (stamps > 0)
        {
            setType(LLT_STAMP);
        }
        else if (texts > 0)
        {
            setType(LLT_FILLING);
        }
    }

    d->doc->setCurrentLayer(this);
    scene->document()->updateLayersStructure();
}

bool LaserLayer::capabaleOf(LaserPrimitiveType primitiveType) const
{
    Q_D(const LaserLayer);
    QList<LaserLayerType> layerTypes = LaserDocument::capableLayerTypeOf(primitiveType);
    return layerTypes.contains(d->type);
}

