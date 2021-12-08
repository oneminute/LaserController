#ifndef IMAGEVIEWER
#define IMAGEVIEWER

#include <QScrollArea>
#include <QLabel>

class ImageViewer : public QScrollArea
{
    Q_OBJECT
public:
    explicit ImageViewer(QWidget* parent = nullptr);
    ~ImageViewer();

    void loadImage(const QString& filename);
    void setImage(const QImage& image);
    void setImage(const QPixmap& image);

    void zoomIn();
    void zoomOut();
    void scaleImage(qreal factor);

    qreal scaleFactor() const;

protected:
    void adjustScrollBar(QScrollBar* scrollBar, double factor);

private:
    QLabel* m_labelImage;
    QPixmap m_image;

    qreal m_scaleFactor;
    qreal m_minScaleFactor;
    qreal m_maxScaleFactor;
};

#endif //IMAGEVIEWER