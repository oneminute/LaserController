#include "TypeUtils.h"
#include <QDateTime>

cv::Point2f typeUtils::qtPointF2CVPoint2f(const QPointF & pt)
{
    return cv::Point2f(pt.x(), pt.y());
}

QString typeUtils::bstrToQString(const _bstr_t & bstr)
{
    return QString::fromUtf16(reinterpret_cast<ushort*>(SysAllocString(bstr)));
}

_bstr_t typeUtils::qStringToBstr(const QString & str)
{
    std::wstring wstr = str.toStdWString();
    _bstr_t bstr = SysAllocString(wstr.c_str());
    return bstr;
}

wchar_t * typeUtils::qStringToWCharPtr(const QString & str)
{
    wchar_t* buf = new wchar_t[str.length() + 1];
    str.toWCharArray(buf);
    buf[str.length()] = 0;
    return buf;
}

char* typeUtils::qStringToCharPtr(const QString& str)
{
    char* buf = new char[str.length() + 1];
    const char* cStr = str.toStdString().c_str();
    memcpy(buf, cStr, str.length());
    buf[str.length()] = 0;
    return buf;
}

cv::Rect typeUtils::qtRect2cvRect(const QRect & rect)
{
    return cv::Rect(rect.left(), rect.top(), rect.right(), rect.bottom());
}

cv::Rect typeUtils::qtRect2cvRect(const QRectF & rect, float scale)
{
    return cv::Rect(
        cv::Point2f(rect.left() * scale, rect.top() * scale), 
        cv::Point2f(rect.right() * scale, rect.bottom() * scale));
}

QVariant typeUtils::stringToVariant(const QString& src, DataType dataType)
{
    QVariant dst;
    switch (dataType)
    {
    case DT_INT:
    {
        dst = src.toInt();
        break;
    }
    case DT_FLOAT:
    {
        dst = src.toFloat();
        break;
    }
    case DT_DOUBLE:
    {
        dst = src.toDouble();
        break;
    }
    case DT_REAL:
    {
        dst = static_cast<qreal>(src.toDouble());
        break;
    }
    case DT_BOOL:
    {
        dst = src.toLower() == "true" || src.toInt() == 1;
        break;
    }
    case DT_STRING:
    {
        dst = src;
        break;
    }
    case DT_DATETIME:
    {
        dst = QDateTime::fromString(src);
        break;
    }
    case DT_RECT:
    case DT_POINT:
    case DT_SIZE:
    {
        break;
    }
    }
    return dst;
}

QString typeUtils::variantToString(const QVariant& src, DataType dataType)
{
    QString dst;
    switch (dataType)
    {
    case DT_INT:
    {
        dst = src.toInt();
        break;
    }
    case DT_FLOAT:
    {
        dst = src.toFloat();
        break;
    }
    case DT_DOUBLE:
    {
        dst = src.toDouble();
        break;
    }
    case DT_REAL:
    {
        dst = static_cast<qreal>(src.toDouble());
        break;
    }
    case DT_BOOL:
    {
        dst = src.toBool() ? "1" : "0";
        break;
    }
    case DT_STRING:
    {
        dst = src.toString();
        break;
    }
    case DT_DATETIME:
    {
        dst = src.toDateTime().toString();
        break;
    }
    case DT_RECT:
    case DT_POINT:
    case DT_SIZE:
    {
        break;
    }
    default:
        dst = src.toString();
        break;
    }
    return dst;
}

QJsonObject typeUtils::point2JsonF(const QPointF& point)
{
    QJsonObject json;
    json["x"] = point.x();
    json["y"] = point.y();
    return json;
}

QJsonObject typeUtils::point2Json(const QPoint& point)
{
    QJsonObject json;
    json["x"] = point.x();
    json["y"] = point.y();
    return json;
}

QPointF typeUtils::json2PointF(const QJsonObject& json)
{
    QPointF point(0, 0);
    if (json.contains("x"))
    {
        point.setX(json["x"].toDouble());
    }
    if (json.contains("y"))
    {
        point.setY(json["y"].toDouble());
    }
    return point;
}

QPoint typeUtils::json2Point(const QJsonObject& json)
{
    QPoint point(0, 0);
    if (json.contains("x"))
    {
        point.setX(json["x"].toInt());
    }
    if (json.contains("y"))
    {
        point.setY(json["y"].toInt());
    }
    return point;
}

QPointF typeUtils::json2PointF(const QJsonValue& json)
{
    return json2Point(json.toObject());
}

QPoint typeUtils::json2Point(const QJsonValue& json)
{
    return json2Point(json.toObject());
}

QJsonObject typeUtils::rect2Json(const QRect& rect)
{
    QJsonObject json;
    int left = rect.left();
    int right = left + rect.width();
    int top = rect.top();
    int bottom = top + rect.height();
    json["x1"] = left;
    json["y1"] = top;
    json["x2"] = right;
    json["y2"] = bottom;
    return json;
}

QImage typeUtils::cvMat2QImage(const cv::Mat& mat)
{
    uchar depth = mat.type() & CV_MAT_DEPTH_MASK;
    uchar channels = 1 + (mat.type() >> CV_CN_SHIFT);
    int rows = mat.rows;
    int cols = mat.cols;

    QImage image;

    QImage::Format format;
    switch (depth) {
    case CV_8U:
        break;
    case CV_8S:
        break;
    case CV_16U:
        break;
    case CV_16S:
        break;
    case CV_32S:
        break;
    case CV_32F:
        break;
    case CV_64F:
        break;
    default:
        break;
    }
    
    return image;
}
