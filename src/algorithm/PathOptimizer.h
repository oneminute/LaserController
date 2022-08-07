#ifndef PATHOPTIMIZER_H
#define PATHOPTIMIZER_H

#include <QObject>
#include <QThread>
#include <QVector>
#include <QVector2D>

#include "laser/LaserPointList.h"
#include "primitive/LaserPrimitiveDeclaration.h"
#include "ui/PreviewWindow.h"

class PathOptimizer;
class OptimizeNode;
class OptimizeEdge;
class ProgressItem;

class PathOptimizerPrivate;
class PathOptimizer : public QObject
{
    Q_OBJECT
public:
    typedef QPair<LaserPrimitive*, int> PathNode;
    typedef QList<OptimizeNode*> Path;

    explicit PathOptimizer(OptimizeNode* root, int totalNodes,
        QObject* parent = nullptr);
    virtual ~PathOptimizer();

    void arriveNode(OptimizeNode* node, QSet<OptimizeNode*>& travelled);

    Path optimizedPath() const;

public slots:
    void optimize(ProgressItem* parentProgress);

signals:
    void finished();
    /**/

protected:
    void optimizeFrom(OptimizeNode* root, ProgressItem* progressParent);

    void optimizeNodes(QSet<OptimizeNode*>& siblingLeaves, QSet<OptimizeNode*>& travelled);

    void printNodeAndEdges();

private:
    QScopedPointer<PathOptimizerPrivate> m_ptr;

    Q_DECLARE_PRIVATE_D(m_ptr, PathOptimizer)
    Q_DISABLE_COPY(PathOptimizer)
};

class Lane : public QSet<OptimizeNode*>
{
public:
    Lane();
    ~Lane();

    void removeNode(OptimizeNode* node);

    void buildKdtree();
    OptimizeNode* nearestSearch(OptimizeNode* node, bool remove = false);

private:
    LaserPointList m_pointList;
};

class LaneMap : public QMap<int, Lane>
{
public:
    LaneMap();
    ~LaneMap();

    void addNode(OptimizeNode* node);
    void addNodes(const QSet<OptimizeNode*>& nodes);
    void removeNode(OptimizeNode* node);

    void buildKdtree();

    Lane& nearestLane(OptimizeNode* node);
    Lane& nearestLane(const LaserPoint& point);

    OptimizeNode* nearestSearch(OptimizeNode* node, bool remove = false, QSet<OptimizeNode*>& externNodes = QSet<OptimizeNode*>());

private:
};

#endif // PATHOPTIMIZER_H