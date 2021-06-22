#include "TypeUtils.h"

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
