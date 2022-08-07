#pragma once

#include <QList>
#include <QPainterPath>
#include <QPoint>
#include <QRect>

struct LaserTextRowPath {
public:
    LaserTextRowPath() {};
    ~LaserTextRowPath() {};

private:
    QPointF m_leftTop;
    QPainterPath m_path;
    QList<QPainterPath> m_subRowPath;
    QList<QRectF> m_boundList;
public:
    QPointF leftTopPosition() { return m_leftTop; };
    QPainterPath path() { return m_path; };
    QList<QPainterPath>& subRowPathlist() { return m_subRowPath; };
    QList<QRectF> subRowBoundList() { return m_boundList; };

    void setLeftTop(QPointF p) { m_leftTop = p; };
    void setPath(QPainterPath p) { m_path = p; };
    void setSubRowPathlist(QList<QPainterPath> l) { m_subRowPath = l; };
    void setSubRowBoundlist(QList<QRectF> l) { m_boundList = l; };
};
