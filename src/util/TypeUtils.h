#ifndef TYPEUTILS_H
#define TYPEUTILS_H

#include <QObject>
#include <QPoint>
#include <comutil.h>

#include <opencv2/opencv.hpp>

namespace typeUtils
{
    cv::Point2f qtPointF2CVPoint2f(const QPointF& pt);

    QString bstrToQString(const _bstr_t& bstr);

    _bstr_t qStringToBstr(const QString& str);
}

#endif // TYPEUTILS_H
