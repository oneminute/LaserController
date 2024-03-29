#ifndef TYPEUTILS_H
#define TYPEUTILS_H

#include <QImage>
#include <QJsonObject>
#include <QObject>
#include <QPoint>
#include <QPointF>
#include <QRect>
#include <QVariant>
#include <QVector3D>

#include "common/common.h"
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

    char* qStringToCharPtr(const QString& str);

    cv::Rect qtRect2cvRect(const QRect& rect);

    cv::Rect qtRect2cvRect(const QRectF& rect, float scale = 1.f);

    QVariant stringToVariant(const QString& src, DataType dataType);

    QString variantToString(const QVariant& src, DataType dataType);

    QJsonObject point2JsonF(const QPointF& point);
    QJsonObject point2Json(const QPoint& point);

    QPointF json2PointF(const QJsonObject& json);
    QPoint json2Point(const QJsonObject& json);

    QPointF json2PointF(const QJsonValue& json);
    QPoint json2Point(const QJsonValue& json);

    QJsonObject vector3D2Json(const QVector3D& point);
    QVector3D json2Vector3D(const QJsonValue& json);
    QVector3D json2Vector3D(const QJsonObject& json);

    QJsonObject size2Json(const QSize& size);
    QSize json2Size(const QJsonValue& json);
    QSize json2Size(const QJsonObject& json);

    QJsonObject rect2Json(const QRect& rect, bool uEnabled = false, bool relative = false);

    QJsonArray pointsToJson(const QList<QPoint>& points);

    QImage cvMat2QImage(const cv::Mat& mat);
}

#endif // TYPEUTILS_H
