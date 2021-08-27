#ifndef PREVIEWVIEWER_H
#define PREVIEWVIEWER_H

#include <QGraphicsView>
#include <QGraphicsPathItem>

class PreviewPathItemPrivate;
class PreviewPathItem : public QGraphicsPathItem
{

};

class PreviewViewerPrivate;
class PreviewViewer : public QGraphicsView
{
    Q_OBJECT
public:
    explicit PreviewViewer(QWidget* parent = nullptr);
    ~PreviewViewer();
};

#endif // PREVIEWVIEWER_H
