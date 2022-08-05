#ifndef STAMPDIALOG_H
#define STAMPDIALOG_H

#include <QDialog>

class LaserScene;
class LaserViewer;
class QGraphicsView;
class LaserLayer;
class LaserPrimitive;

class StampDialog : public QDialog {

    Q_OBJECT
public:
    StampDialog(LaserScene* scene, LaserLayer* layer, QWidget* parent = nullptr);
    virtual ~StampDialog();
    virtual QList<LaserPrimitive*> createStampPrimitive() = 0;

protected slots:
    void okBtnAccept();
    void previewBtnAccept();

protected:
    LaserScene* m_scene;
    LaserViewer* m_viewer;
    QGraphicsView* m_preview;
    LaserLayer* m_layer;
};
#endif