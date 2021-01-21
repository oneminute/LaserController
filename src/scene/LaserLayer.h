#ifndef LASERLAYER_H
#define LASERLAYER_H

#include "common/common.h"
#include <QColor>
#include <QObject>
#include <QPointF>
#include <QList>
#include <QSharedDataPointer>
#include <QExplicitlySharedDataPointer>

class LaserPrimitive;
class LaserLayerPrivate;
class LaserDocument;
class LayerButton;

class LaserLayer : public QObject
{
    Q_OBJECT
public:
    explicit LaserLayer(const QString& name, LaserLayerType type, LaserDocument* document, bool isDefault = false);
    virtual ~LaserLayer();

    bool removable() const { return m_removable; }
    void setRemovable(bool removable) { m_removable = removable; }

    QString id() const { return objectName(); }

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

    int lineSpacing() const;
    void setLineSpacing(int lineSpacing);

    int columnSpacing() const;
    void setColumnSpacing(int columnSpacing);

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

    int lpi() const { return m_lpi; }
    void setLpi(int lpi) { m_lpi = lpi; }

    int dpi() const { return m_dpi; }
    void setDpi(int dpi) { m_dpi = dpi; }

    //qreal nonlinearCoefficient() const { return m_nonlinearCoefficient; }
    //void setNonlinearCoefficient(qreal value) { m_nonlinearCoefficient = value; }

    LaserDocument* document() const;

    void bindButton(LayerButton* button);

    bool exportable() const { return m_exportable; }
    void setExportable(bool value) { m_exportable = value; }

    bool visible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }

    int row() const { return m_row; }
    void setRow(int row) { m_row = row; }

    bool useHalftone() const { return m_useHalftone; }
    void setUseHalftone(bool value) { m_useHalftone = value; }

	bool isDefault() const { return m_isDefault; }

protected:
    void onClicked();

protected:
    bool m_removable;
    LaserLayerType m_type;

    // normal fields
    int m_minSpeed;
    int m_runSpeed;
    int m_laserPower;
    int m_minSpeedPower;
    int m_runSpeedPower;

    // engraving fields
    bool m_engravingForward;
    int m_engravingStyle;
    int m_lineSpacing;
    int m_columnSpacing;
    int m_startX;
    int m_startY;
    int m_errorX;

    // bitmap fields
    int m_lpi;
    int m_dpi;
    int m_row;
    bool m_useHalftone;

    LaserDocument* m_doc;
    LayerButton* m_button;

    bool m_exportable;
    bool m_visible;
	bool m_isDefault;

    QList<LaserPrimitive*> m_items;
    Q_DISABLE_COPY(LaserLayer)
};

#endif // LASERLAYER_H