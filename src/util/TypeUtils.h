#ifndef TYPEUTILS_H
#define TYPEUTILS_H

#include <QObject>
#include <QPoint>
#include <QRect>
#include <comutil.h>

#include <opencv2/opencv.hpp>

namespace typeUtils
{
    template <typename T>
    void arrayDelete(T* buf)
    {
        delete[] buf;
    }

    cv::Point2f qtPointF2CVPoint2f(const QPointF& pt);

    QString bstrToQString(const _bstr_t& bstr);

    _bstr_t qStringToBstr(const QString& str);

    wchar_t* qStringToWCharPtr(const QString& str);

    cv::Rect qtRect2cvRect(const QRect& rect);

    cv::Rect qtRect2cvRect(const QRectF& rect, float scale = 1.f);
}

#endif // TYPEUTILS_H
