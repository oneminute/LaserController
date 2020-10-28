#ifndef UNITUTILS_H
#define UNITUTILS_H

#include "common/common.h"
#include <QPageSize>
#include <QList>

namespace unitUtils
{
    QList<QPageSize::PageSizeId>& presetPageSizes();

    QString pageSizeName(QPageSize::PageSizeId id);

    qreal unitToMM(SizeUnit other);

    inline int mm2MicroMM(double mm)
    {
        return qRound(mm * 1000);
    }
}

#endif // UNITUTILS_H
