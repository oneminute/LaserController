#include "LaserLayer.h"

#include <QMessageBox>
#include <QJsonArray>
#include<QList>

#include "util/Utils.h"
#include "LaserDocument.h"
#include "LaserPrimitive.h"
#include "LaserScene.h"
#include "widget/LayerButton.h"
#include "common/Config.h"
#include "scene/LaserPrimitiveGroup.h"

class LaserLayerPrivate: public ILaserDocumentItemPrivate
{
    Q_DECLARE_PUBLIC(LaserLayer)
public:
    LaserLayerPrivate(LaserLayer* ptr)
        : ILaserDocumentItemPrivate(ptr, LNT_LAYER)
        , removable(true)
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
        , doc(nullptr)
        , lpi(60)
        , dpi(600)
        , button(nullptr)
        , exportable(true)
        , visible(true)
        , row(-1)
        , useHalftone(true)
        , halftoneAngles(Config::EngravingLayer::halftoneAngles())
        //, halftoneGridSize(Config::EngravingLayer::halftoneGridSize())
        , isDefault(false)  
    {}

    bool removable;
    LaserLayerType type;

    // normal fields
    int cuttingRunSpeed;
    qreal cuttingMinSpeedPower;
    qreal cuttingRunSpeedPower;
    int engravingRunSpeed;
    qreal engravingLaserPower;
    qreal engravingMinSpeedPower;
    qreal engravingRunSpeedPower;
    int engravingRowInterval;
    bool engravingEnableCutting;
    int fillingRunSpeed;
    qreal fillingMinSpeedPower;
    qreal fillingRunSpeedPower;
    int fillingRowInterval;
    bool fillingEnableCutting;
    int fillingType;

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
    LayerButton* button;

    bool exportable;
    bool visible;
	bool isDefault;

    QList<LaserPrimitive*> primitives;   
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

	d->useHalftone = Config::EngravingLayer::useHalftone();
	d->lpi = Config::EngravingLayer::LPI();
	d->dpi = Config::EngravingLayer::DPI();
    m_checkBox = box;
}

LaserLayer::~LaserLayer()
{
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

qreal LaserLayer::cuttingMinSpeedPower() const
{
    Q_D(const LaserLayer);
    return d->cuttingMinSpeedPower;
}

void LaserLayer::setCuttingMinSpeedPower(qreal minSpeedPower)
{
    Q_D(LaserLayer);
    d->cuttingMinSpeedPower = minSpeedPower;
}

qreal LaserLayer::cuttingRunSpeedPower() const
{
    Q_D(const LaserLayer);
    return d->cuttingRunSpeedPower;
}

void LaserLayer::setCuttingRunSpeedPower(qreal runSpeedPower)
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

qreal LaserLayer::engravingLaserPower() const
{
    Q_D(const LaserLayer);
    return d->engravingLaserPower;
}

void LaserLayer::setEngravingLaserPower(qreal laserPower)
{
    Q_D(LaserLayer);
    d->engravingLaserPower = laserPower;
}

qreal LaserLayer::engravingMinSpeedPower() const
{
    Q_D(const LaserLayer);
    return d->engravingMinSpeedPower;
}

void LaserLayer::setEngravingMinSpeedPower(qreal minSpeedPower)
{
    Q_D(LaserLayer);
    d->engravingMinSpeedPower = minSpeedPower;
}

qreal LaserLayer::engravingRunSpeedPower() const
{
    Q_D(const LaserLayer);
    return d->engravingRunSpeedPower;
}

void LaserLayer::setEngravingRunSpeedPower(qreal runSpeedPower)
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

qreal LaserLayer::fillingMinSpeedPower() const
{
    Q_D(const LaserLayer);
    return d->fillingMinSpeedPower;
}

void LaserLayer::setFillingMinSpeedPower(qreal minSpeedPower)
{
    Q_D(LaserLayer);
    d->fillingMinSpeedPower = minSpeedPower;
}

qreal LaserLayer::fillingRunSpeedPower() const
{
    Q_D(const LaserLayer);
    return d->fillingRunSpeedPower;
}

void LaserLayer::setFillingRunSpeedPower(qreal runSpeedPower)
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
    int i = this->index();
    item->setLayer(this);
    d->primitives.append(item);
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
	//d->button->setEnabled(true);
    connect(button, &LayerButton::clicked, this, &LaserLayer::onClicked);
	//m_index = index;
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

QRectF LaserLayer::boundingRect() const
{
    Q_D(const LaserLayer);
    return utils::boundingRect(d->primitives);
}

QPointF LaserLayer::position() const 
{
    return boundingRect().topLeft();
}

QPointF LaserLayer::positionMM() const
{
    return Global::matrixToMM(SU_PX).map(position());
}

QPointF LaserLayer::positionMachining() const
{
    return Global::matrixToMachining().map(position());
}

QCheckBox * LaserLayer::checkBox()
{
    return m_checkBox;
}

void LaserLayer::setCheckBox(QCheckBox * box)
{
    m_checkBox = box;
}

qreal LaserLayer::accelerationLength(LaserLayerType layerType) const
{
    Q_D(const LaserLayer);
    qreal minSpeed = Config::UserRegister::scanXStartSpeed();
    qreal acc = Config::UserRegister::scanXAcc();
    //qreal maxSpeed = i.value()->layer()->engravingRunSpeed() * 1000;
    //qreal span = (maxSpeed * maxSpeed - minSpeed * minSpeed) / (acc * 2);
    switch (layerType)
    {
    case LLT_CUTTING:
        //minSpeed = d->min
        break;
    case LLT_ENGRAVING:
        break;
    case LLT_FILLING:
        break;
    }
    return 0;
}

qreal LaserLayer::accelerationSegmentLength(LaserLayerType layerType) const
{
    return qreal();
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

       /* QList<LaserPrimitiveType> types;
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
            else if (primitive->isText()) {
                if (!types.contains(LPT_TEXT))
                {
                    types.append(LPT_TEXT);
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
        else if (types[0] == LPT_TEXT)
        {
            setType(LLT_CUTTING);
        }*/
        scene->document()->updateLayersStructure();
    }
}

