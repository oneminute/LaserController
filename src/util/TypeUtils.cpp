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
