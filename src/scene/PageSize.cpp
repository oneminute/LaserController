#include "PageSize.h"

PageSize::PageSize()
    : m_name("")
    , m_width(0)
    , m_height(0)
{
}

PageSize::PageSize(const QString & _name, qreal _width, qreal _height)
    : m_name(_name)
    , m_width(_width)
    , m_height(_height)
{
}

QString PageSize::toString()
{
    return QString("%1 (%2 x %3)").arg(m_name).arg(m_width).arg(m_height);
}

QMap<int, PageSize>& PageSize::presets()
{
    static QMap<int, PageSize> PageSizeList
    {
    {PT_A0, PageSize( "A0", 841, 1189)},
    {PT_A1, PageSize( "A1", 594, 841)},
    {PT_A2, PageSize( "A2", 420, 594)},
    {PT_A3, PageSize( "A3", 297, 420)},
    {PT_A4, PageSize( "A4", 210, 297)},
    {PT_A5, PageSize( "A5", 148, 210)},
    {PT_A6, PageSize( "A6", 105, 148)},
    {PT_A7, PageSize( "A7", 74, 105)},
    {PT_A8, PageSize( "A8", 52, 74)},
    {PT_A9, PageSize( "A9", 37, 52)},
    {PT_A10, PageSize( "A10", 26, 37)}
    };
    return PageSizeList;
}