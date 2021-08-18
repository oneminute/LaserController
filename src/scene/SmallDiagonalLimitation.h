#ifndef SMALLDIAGONALLIMITATION_H
#define SMALLDIAGONALLIMITATION_H

#include <QObject>
#include <QDebug>
#include <QJsonObject>

struct SmallDiagonalLimitationItem
{
public:
    SmallDiagonalLimitationItem()
        : diagonal(0)
        , laserPower(0)
        , speed(0)
    {}

    QJsonObject toJson() const;

    qreal diagonal;
    qreal laserPower;
    qreal speed;
};

bool operator<(const SmallDiagonalLimitationItem& v1, const SmallDiagonalLimitationItem& v2);
bool operator==(const SmallDiagonalLimitationItem& v1, const SmallDiagonalLimitationItem& v2);

QDebug operator<<(QDebug dbg, const SmallDiagonalLimitationItem& item);

class SmallDiagonalLimitation
{
public:
    typedef QList<SmallDiagonalLimitationItem> ItemList;
    SmallDiagonalLimitation();
    SmallDiagonalLimitation(const ItemList& list);

    ~SmallDiagonalLimitation() {}

    qreal maxDiagonal() const;

    ItemList& items();
    void addItem(const SmallDiagonalLimitationItem& item);

    QJsonObject toJson() const;

private:
    ItemList m_items;
};

QDebug operator<<(QDebug dbg, const SmallDiagonalLimitation& limitation);

Q_DECLARE_METATYPE(SmallDiagonalLimitation);

#endif // SMALLDIAGONALLIMITATION_H
