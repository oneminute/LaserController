#ifndef LASERLAYER_H
#define LASERLAYER_H

#include "common/common.h"
#include <QObject>
#include <QPointF>
#include <QList>
#include <QSharedDataPointer>
#include <QExplicitlySharedDataPointer>

class LaserItem;
class LaserLayerPrivate;
class LaserDocument;

class LaserLayer : public QObject
{
    Q_OBJECT
public:
    explicit LaserLayer(LaserDocument* document);
    LaserLayer(const QString& id, LayerType type, LaserDocument* document);
    ~LaserLayer();

    QString id() const;
    void setId(const QString& id);
    LayerType type() const;

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

    int errorY() const;
    void setErrorY(int errorY);

    int moveSpeed() const;
    void setMoveSpeed(int moveSpeed);

    int minSpeedPower() const;
    void setMinSpeedPower(int minSpeedPower);

    int runSpeedPower() const;
    void setRunSpeedPower(int runSpeedPower);

    void addItem(LaserItem* item);
    QList<LaserItem*>& items();

    LaserDocument* document() const;

protected:
    LayerType m_type;

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

    LaserDocument* m_doc;

    QList<LaserItem*> m_items;
    Q_DISABLE_COPY(LaserLayer)
};

#endif // LASERLAYER_H