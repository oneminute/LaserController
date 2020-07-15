#ifndef LASERLAYER_H
#define LASERLAYER_H

#include <QObject>
#include <QPointF>
#include <QList>
#include <QSharedDataPointer>
#include <QExplicitlySharedDataPointer>

class LaserItem;
class LaserLayerPrivate;

class LaserLayer
{
public:
    enum LayerType
    {
        LLT_ENGRAVING,
        LLT_CUTTING
    };

    LaserLayer();
    LaserLayer(const QString& id, LayerType type);
    LaserLayer(const LaserLayer& other);
    ~LaserLayer();

    LaserLayer& operator=(const LaserLayer& other);

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

protected:
    QExplicitlySharedDataPointer<LaserLayerPrivate> d_ptr;
    friend class LaserLayerPrivate;
};

#endif // LASERLAYER_H