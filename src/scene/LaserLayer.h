#ifndef LASERLAYER_H
#define LASERLAYER_H

#include "common/common.h"
#include <QColor>
#include <QObject>
#include <QPointF>
#include <QList>
#include <QSharedDataPointer>
#include <QExplicitlySharedDataPointer>
#include <QJsonObject>
#include <QCheckBox>
#include "scene/LaserDocumentItem.h"

class LaserPrimitive;
class LaserLayerPrivate;
class LaserDocument;
class LayerButton;

class LaserLayerPrivate;
class LaserLayer : public QObject, public ILaserDocumentItem
{
    Q_OBJECT
public:
    explicit LaserLayer(const QString& name, LaserLayerType type, LaserDocument* document, bool isDefault = false, QCheckBox* box = nullptr);
    virtual ~LaserLayer();

    void init();

    bool removable() const;
    void setRemovable(bool removable);

    QString name() const;
    void setName(const QString& value);
    LaserLayerType type() const;
    void setType(LaserLayerType type);

    int cuttingRunSpeed() const;
    void setCuttingRunSpeed(int runSpeed);

    int cuttingMinSpeedPower() const;
    void setCuttingMinSpeedPower(int minSpeedPower);

    int cuttingRunSpeedPower() const;
    void setCuttingRunSpeedPower(int runSpeedPower);

    int engravingRunSpeed() const;
    void setEngravingRunSpeed(int runSpeed);

    int engravingLaserPower() const;
    void setEngravingLaserPower(int laserPower);

    int engravingMinSpeedPower() const;
    void setEngravingMinSpeedPower(int minSpeedPower);

    int engravingRunSpeedPower() const;
    void setEngravingRunSpeedPower(int runSpeedPower);

    bool engravingForward() const;
    void setEngravingForward(bool engravingForward);

    int engravingStyle() const;
    void setEngravingStyle(int engravingStyle);

    int engravingRowInterval() const;
    void setEngravingRowInterval(int rowInterval);

    bool engravingEnableCutting() const;
    void setEngravingEnableCutting(bool cutting);

    int fillingRunSpeed() const;
    void setFillingRunSpeed(int runSpeed);

    int fillingMinSpeedPower() const;
    void setFillingMinSpeedPower(int minSpeedPower);

    int fillingRunSpeedPower() const;
    void setFillingRunSpeedPower(int runSpeedPower);

    int fillingRowInterval() const;
    void setFillingRowInterval(int rowInterval);

    bool fillingEnableCutting() const;
    void setFillingEnableCutting(bool cutting);

    int fillingType() const;
    void setFillingType(int type);

    int stampBoundingDistance() const;
    void setStampBoundingDistance(int distance);

    QPoint startPos() const;
    void setStartPos(const QPoint& startPos);

    int startX() const;
    void setStartX(int x);

    int startY() const;
    void setStartY(int y);

    int errorX() const;
    void setErrorX(int errorX);

    QList<LaserPrimitive*> primitives();
    LaserPrimitive* primitiveById(const QString& id) const;
    bool isEmpty() const;
    int count() const;
    bool contains(const QString& id) const;
    bool contains(LaserPrimitive* primitive) const;

    QColor color() const;
    void setColor(const QColor& value);

    int lpi() const;
    void setLpi(int lpi);

    int dpi() const;
    void setDpi(int dpi);

    LaserDocument* document() const;

    bool exportable() const;
    void setExportable(bool value);

    bool visible() const;
    void setVisible(bool visible);

    int row() const;
    void setRow(int row);

    bool useHalftone() const;
    void setUseHalftone(bool value);

    qreal halftoneAngles() const;
    void setHalftoneAngles(qreal angles);

    //bool isDefault() const;

    virtual bool isAvailable() const;

	virtual QJsonObject toJson(QWidget* window);

	int index();

	void setIndex(int i);

    QRect boundingRect() const;

    virtual QPoint position() const;

    QCheckBox* checkBox();

    void setCheckBox(QCheckBox* box);

    void setSelected();

    bool capabaleOf(LaserPrimitiveType primitiveType) const;

protected:
    void addPrimitive(LaserPrimitive* item);
    void removePrimitive(LaserPrimitive* item, bool release);
    void removePrimitiveById(const QString& id, bool release);

private:
	int m_index;
    QCheckBox* m_checkBox;

    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserLayer);
    Q_DISABLE_COPY(LaserLayer)

    friend class LaserDocument;
};

#endif // LASERLAYER_H