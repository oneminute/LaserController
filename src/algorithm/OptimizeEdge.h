#ifndef OPTIMIZEEDGE_H
#define OPTIMIZEEDGE_H

#include <QObject>

class OptimizeNode;

class OptimizeEdgePrivate;
class OptimizeEdge
{
public:
    explicit OptimizeEdge(OptimizeNode* a, OptimizeNode* b, bool force = false, bool forward = false);
    ~OptimizeEdge();

    void clear();

    double length() const;
    void setLength(double length);
    void setLength(const QPointF& p1, const QPointF& p2);

    double weight() const;
    void setWeight(double weight);

    OptimizeNode* a();
    OptimizeNode* b();

    void print();

    QLineF toLine() const;

private:
    QScopedPointer<OptimizeEdgePrivate> d_ptr;

    Q_DECLARE_PRIVATE(OptimizeEdge)
    Q_DISABLE_COPY(OptimizeEdge)
};

#endif // OPTIMIZEEDGE_H