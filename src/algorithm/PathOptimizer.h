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
    void arrived(OptimizeNode* node, const LaserPoint& lastPos);
    OptimizeNode* currentNode() const;
    LaserPoint currentPos() const;
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
    typedef QList<OptimizeNode*> Path;
    explicit PathOptimizer(OptimizeNode* root, int totalNodes,
        QObject* parent = nullptr);
    virtual ~PathOptimizer();

    QList<OptimizeEdge*> edges() const;
    int edgesCount() const;
    Path optimizedPath() const;

    void setCanvas(cv::Mat& canvas);


public slots:
    void optimize(int canvasWidth, int canvasHeight);

signals:
    void progressUpdated(float progress);
    void messageUpdated(const QString& msg);
    void titleUpdated(const QString& msg);
    void finished();

    void drawPrimitive(LaserPrimitive* primitive);
    void drawPath(const QPainterPath& path, QPen pen = QPen(Qt::blue, 1, Qt::SolidLine), const QString& label = QString());
    void drawLine(const QLineF& line, QPen pen = QPen(Qt::black, 1, Qt::SolidLine),
        const QString& label = QString());

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