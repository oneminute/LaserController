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

    bool removable() const;
    void setRemovable(bool removable);

    QString name() const;
    LaserLayerType type() const;
    void setType(LaserLayerType type);

    int cuttingRunSpeed() const;
    void setCuttingRunSpeed(int runSpeed);

    qreal cuttingMinSpeedPower() const;
    void setCuttingMinSpeedPower(qreal minSpeedPower);

    qreal cuttingRunSpeedPower() const;
    void setCuttingRunSpeedPower(qreal runSpeedPower);

    int engravingRunSpeed() const;
    void setEngravingRunSpeed(int runSpeed);

    qreal engravingLaserPower() const;
    void setEngravingLaserPower(qreal laserPower);

    qreal engravingMinSpeedPower() const;
    void setEngravingMinSpeedPower(qreal minSpeedPower);

    qreal engravingRunSpeedPower() const;
    void setEngravingRunSpeedPower(qreal runSpeedPower);

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

    qreal fillingMinSpeedPower() const;
    void setFillingMinSpeedPower(qreal minSpeedPower);

    qreal fillingRunSpeedPower() const;
    void setFillingRunSpeedPower(qreal runSpeedPower);

    int fillingRowInterval() const;
    void setFillingRowInterval(int rowInterval);

    bool fillingEnableCutting() const;
    void setFillingEnableCutting(bool cutting);

    QPoint startPos() const;
    void setStartPos(const QPoint& startPos);

    int startX() const;
    void setStartX(int x);

    int startY() const;
    void setStartY(int y);

    int errorX() const;
    void setErrorX(int errorX);

    void addPrimitive(LaserPrimitive* item);
    QList<LaserPrimitive*>& primitives();
    void removePrimitive(LaserPrimitive* item);
    bool isEmpty() const;

    QColor color() const;

    int lpi() const;
    void setLpi(int lpi);

    int dpi() const;
    void setDpi(int dpi);

    //qreal nonlinearCoefficient() const { return m_nonlinearCoefficient; }
    //void setNonlinearCoefficient(qreal value) { m_nonlinearCoefficient = value; }

    LaserDocument* document() const;

    void bindButton(LayerButton* button, int index);

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

    //int halftoneGridSize() const;
    //void setHalftoneGridSize(int gridSize);

    bool isDefault() const;

    virtual bool isAvailable() const;

	virtual QJsonObject toJson(QWidget* window);

	int index();

	void setIndex(int i);

    QRectF boundingRect() const;

    virtual QPointF position() const;
    virtual QPointF positionMM() const;
    virtual QPointF positionMachining() const;

    QCheckBox* checkBox();

    void setCheckBox(QCheckBox* box);

protected:
    void onClicked();

protected:
    
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserLayer);
    Q_DISABLE_COPY(LaserLayer)
private:
	int m_index;
    QCheckBox* m_checkBox;
};

#endif // LASERLAYER_H