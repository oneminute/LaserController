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

class AntPrivate;
class Ant
{
public:
    explicit Ant(int antIndex, PathOptimizer* optimizer);

    void initialize();
    void arrived(OptimizeNode* node, const QPointF& lastPos);
    OptimizeNode* currentNode() const;
    QPointF currentPos() const;
    bool moveForward();
    int antIndex() const;
    double totalLength() const;
    void drawPath(cv::Mat& canvas, int iteration);
    QQueue<OptimizeNode*> path() const;
    QMap<OptimizeNode*, int> arrivedNodes() const;

private:
    QScopedPointer<AntPrivate> m_ptr;
    Q_DECLARE_PRIVATE_D(m_ptr, Ant)
    Q_DISABLE_COPY(Ant)
};

class PathOptimizerPrivate;
class PathOptimizer : public QObject
{
    Q_OBJECT
public:
    typedef QPair<LaserPrimitive*, int> PathNode;
    typedef QList<PathNode> Path;
    explicit PathOptimizer(OptimizeNode* root, int totalNodes, int maxIterations, 
        int maxAnts, int maxTravers, float volatileRate, 
        bool useGreedyAlgorithm = false, bool containsLayers = true,
        QObject* parent = nullptr);
    virtual ~PathOptimizer();

    bool isContainsLayers() const;

    QList<OptimizeEdge*> edges() const;
    int edgesCount() const;
    bool useGreedyAlgorithm() const;
    Path optimizedPath() const;

    void setCanvas(cv::Mat& canvas);


public slots:
    void optimize(int canvasWidth, int canvasHeight);

signals:
    void progressUpdated(float progress);
    void messageUpdated(const QString& msg);
    void titleUpdated(const QString& msg);
    void finished();

protected:
    void initializeByGroups(OptimizeNode* root);

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
    OptimizerController(OptimizeNode* root, int totalNodes, QObject* parent = nullptr);
    ~OptimizerController();

    PathOptimizer::Path optimize(float pageWidth, float pageHeight, cv::Mat& canvas = cv::Mat());

public slots:
    void finished();

signals:
    void start(float pageWidth, float pageHeight);

private:
    //QScopedPointer<ProgressDialog> m_dialog;
    QScopedPointer<PreviewWindow> m_previewWindow;
    QThread m_thread;
    QScopedPointer<PathOptimizer> m_optimizer;
};

void drawPath(cv::Mat& canvas, const QQueue<OptimizeNode*>& path, const QMap<OptimizeNode*, int>& arrivedNodes);

#endif // PATHOPTIMIZER_H