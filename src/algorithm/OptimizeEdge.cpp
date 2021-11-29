#include "OptimizeEdge.h"

#include "OptimizeNode.h"
#include <QVector2D>
#include <QVector4D>

class OptimizeEdgePrivate
{
    Q_DECLARE_PUBLIC(OptimizeEdge)
public:
    OptimizeEdgePrivate(OptimizeEdge* ptr)
        : q_ptr(ptr)
        , a(nullptr)
        , b(nullptr)
        , length(0)
        , weight(1)
        , force(false)
        , forward(false)
    {
    }

    ~OptimizeEdgePrivate()
    {}

    OptimizeNode* a;
    OptimizeNode* b;

    double length;
    double weight;

    bool force;
    bool forward;
    OptimizeEdge* q_ptr;
};

OptimizeEdge::OptimizeEdge(OptimizeNode* a, OptimizeNode* b, bool force, bool forward)
    : d_ptr(new OptimizeEdgePrivate(this))
{
    Q_D(OptimizeEdge);
    d->a = a;
    d->b = b;
    d->force = force;
    d->forward = forward;

    d->length = QVector2D(a->startPos().toPoint() - b->startPos().toPoint()).length();
}

OptimizeEdge::~OptimizeEdge()
{
}

void OptimizeEdge::clear()
{

}

double OptimizeEdge::length() const
{
    Q_D(const OptimizeEdge);
    return d->length;
}

void OptimizeEdge::setLength(double length)
{
    Q_D(OptimizeEdge);
    d->length = length;
}

void OptimizeEdge::setLength(const QPointF& p1, const QPointF& p2)
{
    setLength(QVector2D(p1 - p2).length());
}

double OptimizeEdge::weight() const
{
    Q_D(const OptimizeEdge);
    return d->weight;
}

void OptimizeEdge::setWeight(double weight)
{
    Q_D(OptimizeEdge);
    d->weight = weight;
}

OptimizeNode* OptimizeEdge::a()
{
    Q_D(OptimizeEdge);
    return d->a;
}

OptimizeNode* OptimizeEdge::b()
{
    Q_D(OptimizeEdge);
    return d->b;
}

void OptimizeEdge::print()
{
    Q_D(OptimizeEdge);
    qLogD << "    " << d->a->nodeName() << " --> " << d->b->nodeName() 
        << ", length = " << d->length << ", weight = " << d->weight;
}

QLine OptimizeEdge::toLine() const
{
    Q_D(const OptimizeEdge);
    return QLine(d->a->currentPos().toPoint(), d->b->currentPos().toPoint());
}

