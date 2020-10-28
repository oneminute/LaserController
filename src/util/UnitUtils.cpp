#include "UnitUtils.h"
#include "UnitUtils.h"

#include <QtMath>
#include <QSize>

namespace unitUtils
{
    QList<QPageSize::PageSizeId>& presetPageSizes() {
        static QList<QPageSize::PageSizeId> presets
        {
            QPageSize::A0,
            QPageSize::A1,
            QPageSize::A2,
            QPageSize::A3,
            QPageSize::A4,
            QPageSize::A5,
            QPageSize::A6,
            QPageSize::A7,
            QPageSize::A8,
            QPageSize::A9,
            QPageSize::A10,
            QPageSize::B0,
            QPageSize::B1,
            QPageSize::B2,
            QPageSize::B3,
            QPageSize::B4,
            QPageSize::B5,
            QPageSize::B6,
            QPageSize::B7,
            QPageSize::B8,
            QPageSize::B9,
            QPageSize::B10,
            QPageSize::Letter,
            QPageSize::Legal,
            QPageSize::Ledger,
            QPageSize::Tabloid
        };
        return presets;
    }

    QString pageSizeName(QPageSize::PageSizeId id)
    {
        QPageSize ps(id);
        QString item = QString("%1 (%2 x %3)").arg(ps.name()).arg(ps.size(QPageSize::Unit::Millimeter).width()).arg(ps.size(QPageSize::Unit::Millimeter).height());
        return item;
    }
    qreal unitToMM(SizeUnit other)
    {
        qreal ratio = 1.0f;
        switch (other)
        {
        case SU_MM:
            ratio = 1.f;
            break;
        case SU_MM100:
            ratio = 0.01f;
            break;
        case SU_CM:
            ratio = 10.f;
            break;
        default:
            break;
        }
        return ratio;
    }

}
