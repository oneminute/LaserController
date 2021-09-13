#ifndef PATHOPTIMIZER_H
#define PATHOPTIMIZER_H

#include <QObject>
#include <QThread>
#include <QVector>
#include <QVector2D>
#include "scene/LaserPrimitive.h"
#include "ui/ProgressDialog.h"
#include "ui/PreviewWindow.h"

class PathOptimizer;
class OptimizeNode;
class OptimizeEdge;

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

    Path optimizedPath() const;

public slots:
    void optimize();

signals:
    void finished();
    /**/

protected:
    void optimizeLayer(OptimizeNode* root);

    void printNodeAndEdges();

private:
    QScopedPointer<PathOptimizerPrivate> m_ptr;

    Q_DECLARE_PRIVATE_D(m_ptr, PathOptimizer)
    Q_DISABLE_COPY(PathOptimizer)
};

class OptimizerController : public QObject
{
    Q_OBJECT
public:
    typedef std::function<void(OptimizerController*)> FinishedCallback;
    OptimizerController(OptimizeNode* root, int totalNodes, QObject* parent = nullptr);
    ~OptimizerController();

    void optimize();

    PathOptimizer::Path path();

    void setFinishedCallback(FinishedCallback callback);

public slots:
    void finished();

signals:
    void start();

private:
    QThread m_thread;
    FinishedCallback m_finishedCallback;
    QScopedPointer<PathOptimizer> m_optimizer;
};

#endif // PATHOPTIMIZER_H