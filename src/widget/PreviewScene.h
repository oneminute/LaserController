#ifndef PREVIEWSCENE_H
#define PREVIEWSCENE_H

#include <QGraphicsScene>

class PreviewScenePrivate;
class PreviewScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit PreviewScene(QObject* parent = nullptr);
    ~PreviewScene();

    QPen& labelPen();
    void setLabelPen(QPen& pen);

    QFont& labelFont();
    void setLabelFont(QFont& font);

    void reset();

public slots:
    void addPath(const QPainterPath& path, QPen pen = QPen(Qt::blue, 1, Qt::SolidLine), const QString& label = QString());
    void addLine(const QLineF& line, QPen pen = QPen(Qt::black, 1, Qt::SolidLine), const QString& label = QString());
    void addPoints(const QList<QPointF>& points, QPen pen = QPen(Qt::gray, 0.5, Qt::SolidLine), int style = 0);
    void addPoint(const QPointF& point, QPen pen = QPen(Qt::gray, 0.5, Qt::SolidLine), int style = 0);

private:
    QScopedPointer<PreviewScenePrivate> d_ptr;

    Q_DECLARE_PRIVATE(PreviewScene)
    Q_DISABLE_COPY(PreviewScene)
};

#endif // PREVIEWSCENE_H