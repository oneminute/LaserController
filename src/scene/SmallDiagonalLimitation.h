#ifndef SMALLDIAGONALLIMITATION_H
#define SMALLDIAGONALLIMITATION_H

#include <QObject>
#include <QDebug>
#include <QJsonObject>
#include <QList>
#include <QMap>

struct SmallDiagonalLimitationItem
{
public:
    SmallDiagonalLimitationItem(qreal _diagonal = 0, qreal _laserPower = 0, qreal _speed = 0)
        : diagonal(_diagonal)
        , laserPower(_laserPower)
        , speed(_speed)
    {}

    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);

    qreal diagonal;
    qreal laserPower;
    qreal speed;
};

bool operator<(const SmallDiagonalLimitationItem& v1, const SmallDiagonalLimitationItem& v2);
bool operator==(const SmallDiagonalLimitationItem& v1, const SmallDiagonalLimitationItem& v2);

QDebug operator<<(QDebug dbg, const SmallDiagonalLimitationItem& item);

typedef QMap<qreal, SmallDiagonalLimitationItem> LimitationMap;

class SmallDiagonalLimitation : public QMap<qreal, SmallDiagonalLimitationItem>
{
public:
    SmallDiagonalLimitation();
    SmallDiagonalLimitation(const SmallDiagonalLimitation& limitation);

    ~SmallDiagonalLimitation() {}

    qreal maxDiagonal() const;

    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);

    SmallDiagonalLimitationItem& createNewItem();

    int indexOf(qreal diagonal) const;
};

QDebug operator<<(QDebug dbg, const SmallDiagonalLimitation& limitation);

Q_DECLARE_METATYPE(SmallDiagonalLimitation);
Q_DECLARE_METATYPE(SmallDiagonalLimitation*);

#endif // SMALLDIAGONALLIMITATION_H
