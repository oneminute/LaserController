#pragma once

#include "LaserStampBasePrivate.h"

class LaserStampTextPrivate : public LaserStampBasePrivate
{
    Q_DECLARE_PUBLIC(LaserStampText)
public:
    LaserStampTextPrivate(LaserStampText* ptr)
        : LaserStampBasePrivate(ptr)
    {
    }
    QString content;
    QSize size;
    qreal space;
    bool bold;
    bool italic;
    bool uppercase;
    qreal weight;
    QString family;
    qreal fontPiexlSize;
    QPainterPath stampPath;
    
};

