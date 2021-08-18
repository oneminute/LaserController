#include "SmallDiagonalLimitation.h"

#include <QJsonArray>

QJsonObject SmallDiagonalLimitationItem::toJson() const
{
    QJsonObject json;
    json.insert("diagonal", diagonal);
    json.insert("laserPower", laserPower);
    json.insert("speed", speed);
    return json;
}

bool operator<(const SmallDiagonalLimitationItem& v1, const SmallDiagonalLimitationItem& v2)
{
    return v1.diagonal < v2.diagonal;
}

bool operator==(const SmallDiagonalLimitationItem& v1, const SmallDiagonalLimitationItem& v2)
{
    return qFuzzyCompare(v1.diagonal, v2.diagonal) &&
        qFuzzyCompare(v1.laserPower, v2.laserPower) &&
        qFuzzyCompare(v1.speed, v2.speed);
}

QDebug operator<<(QDebug dbg, const SmallDiagonalLimitationItem& item)
{
    dbg.nospace() << "[diagonal = " << item.diagonal <<
        ", laserPower = " << item.laserPower <<
        ", speed = " << item.speed << "]";
    return dbg.maybeSpace();
}

SmallDiagonalLimitation::SmallDiagonalLimitation()
{

}

SmallDiagonalLimitation::SmallDiagonalLimitation(const ItemList& list)
    : m_items(list)
{
}

qreal SmallDiagonalLimitation::maxDiagonal() const
{
    if (m_items.isEmpty())
    {
        return 0;
    }
    else
    {
        return m_items.first().diagonal;
    }
}

SmallDiagonalLimitation::ItemList& SmallDiagonalLimitation::items()
{
    return m_items;
}

void SmallDiagonalLimitation::addItem(const SmallDiagonalLimitationItem& item)
{
    m_items.append(item);
    qSort(m_items);
}

QJsonObject SmallDiagonalLimitation::toJson() const
{
    QJsonArray array;
    for (const SmallDiagonalLimitationItem& item : m_items)
    {
        array.append(item.toJson());
    }
    QJsonObject json;
    json.insert("items", array);
    return json;
}

QDebug operator<<(QDebug dbg, SmallDiagonalLimitation& limitation)
{
    dbg.nospace() << "size of limitation: " << limitation.items().size();
    for (const SmallDiagonalLimitationItem& item : limitation.items())
    {
        dbg.nospace() << "  " << item;
    }
    return dbg.maybeSpace();
}

