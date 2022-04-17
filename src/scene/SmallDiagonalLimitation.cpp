#include "SmallDiagonalLimitation.h"

#include "common/common.h"
#include "common/Config.h"
#include <QJsonArray>

QJsonObject SmallDiagonalLimitationItem::toJson() const
{
    QJsonObject json;
    json.insert("diagonal", qRound(diagonal * 1000));
    json.insert("laserPower", qRound(laserPower * 10));
    json.insert("speed", qRound(speed * 1000));
    return json;
}

void SmallDiagonalLimitationItem::fromJson(const QJsonObject& json)
{
    if (json.contains("diagonal"))
    {
        diagonal = json["diagonal"].toDouble() / 1000.0;
    }
    if (json.contains("laserPower"))
    {
        laserPower = json["laserPower"].toDouble() / 10.0;
    }
    if (json.contains("speed"))
    {
        speed = json["speed"].toDouble() / 1000.0;
    }
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

SmallDiagonalLimitation::SmallDiagonalLimitation(const SmallDiagonalLimitation& list)
    : QMap<qreal, SmallDiagonalLimitationItem>(list)
{
}

qreal SmallDiagonalLimitation::maxDiagonal() const
{
    if (Config::Export::enableSmallDiagonal())
    {
        if (isEmpty())
        {
            return 0;
        }
        else
        {
            return first().diagonal;
        }
    }
    else
    {
        return 0;
    }
}

QJsonObject SmallDiagonalLimitation::toJson() const
{
    QJsonArray array;
    for (LimitationMap::ConstIterator i = constBegin(); i != constEnd(); i++)
    {
        array.append(i.value().toJson());
    }
    QJsonObject json;
    json.insert("items", array);
    return json;
}

void SmallDiagonalLimitation::fromJson(const QJsonObject& json)
{
    if (json.contains("items"))
    {
        clear();
        QJsonArray array = json["items"].toArray();
        for (const QJsonValue& value : array)
        {
            SmallDiagonalLimitationItem item;
            item.fromJson(value.toObject());
            insert(item.diagonal, item);
        }
    }
}

SmallDiagonalLimitationItem& SmallDiagonalLimitation::createNewItem()
{
    qreal diagonal = 2;
    qreal power = 10;
    qreal speed = 5;
    if (!isEmpty())
    {
        diagonal = last().diagonal + 1;
        power = last().laserPower;
        speed = last().speed;
    }
    SmallDiagonalLimitationItem item = { diagonal, power, speed };
    insert(diagonal, item);
    return last();
}

int SmallDiagonalLimitation::indexOf(qreal diagonal) const
{
    QList<qreal> keyList = keys();
    LimitationMap::ConstIterator upperIt = upperBound(diagonal);
    if (upperIt == constEnd())
        return -1;
    return keyList.indexOf(upperIt.key());
}

QDebug operator<<(QDebug dbg, const SmallDiagonalLimitation& limitation)
{
    dbg.nospace() << "size of limitation: " << limitation.size();
    for (LimitationMap::ConstIterator i = limitation.constBegin(); i != limitation.constEnd(); i++)
    {
        dbg.nospace() << "  " << i.value();
    }
    return dbg.maybeSpace();
}

