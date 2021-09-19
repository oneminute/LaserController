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
    explicit LaserLayer(const QString& name, LaserLayerType type, LaserDocument* document, bool isDefault = false);
    virtual ~LaserLayer();

    bool removable() const;
    void setRemovable(bool removable);

    QString name() const;
    LaserLayerType type() const;
    void setType(LaserLayerType type);

    int minSpeed() const;
    void setMinSpeed(int minSpeed);

    int runSpeed() const;
    void setRunSpeed(int runSpeed);

    int laserPower() const;
    void setLaserPower(int laserPower);

    bool engravingForward() const;
    void setEngravingForward(bool engravingForward);

    int engravingStyle() const;
    void setEngravingStyle(int engravingStyle);

    int rowInterval() const;
    void setRowInterval(int rowInterval);

    QPoint startPos() const;
    void setStartPos(const QPoint& startPos);

    int startX() const;
    void setStartX(int x);

    int startY() const;
    void setStartY(int y);

    int errorX() const;
    void setErrorX(int errorX);

    int minSpeedPower() const;
    void setMinSpeedPower(int minSpeedPower);

    int runSpeedPower() const;
    void setRunSpeedPower(int runSpeedPower);

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

    bool isDefault() const;

    virtual bool isAvailable() const;

	virtual QJsonObject toJson(QWidget* window);

	int index();

	void setIndex(int i);

    QRectF boundingRect() const;

    virtual QPointF position() const;
    virtual QPointF positionMM() const;
    virtual QPointF positionMachining() const;

protected:
    void onClicked();

protected:
    
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserLayer);
    Q_DISABLE_COPY(LaserLayer)
private:
	int m_index;
};

#endif // LASERLAYER_H