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
    QImage image() const;
    QPixmap pixmap() const;

    void zoomIn();
    void zoomOut();
    void scaleImage(qreal factor);
    void fit();
    void fitBy(const QSize& size);

    qreal scaleFactor() const;
    void setScaleFactor(qreal value);

protected:
    void adjustScrollBar(QScrollBar* scrollBar, double factor);

    virtual bool event(QEvent* e) override;
    virtual void wheelEvent(QWheelEvent* e) override;
    virtual void mouseMoveEvent(QMouseEvent* e) override;

protected slots:


private:
    QLabel* m_labelImage;
    QPixmap m_image;

    bool m_ctrlPressed;

    qreal m_scaleFactor;
    qreal m_minScaleFactor;
    qreal m_maxScaleFactor;

};

#endif //IMAGEVIEWER