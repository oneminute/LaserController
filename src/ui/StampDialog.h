#ifndef STAMPDIALOG_H
#define STAMPDIALOG_H
#include <QDialog>
#include "scene/LaserScene.h"
#include "widget/LaserViewer.h"

class StampDialog : public QDialog {

    Q_OBJECT
public:
    StampDialog(LaserScene* scene, QWidget* parent = nullptr);
    virtual ~StampDialog();
    virtual QList<LaserPrimitive*> createStampPrimitive() = 0;
    protected slots:
    void okBtnAccept();
    void previewBtnAccept();
protected:
    LaserScene* m_scene;
    LaserViewer* m_viewer;
    QGraphicsView* m_preview;
};
#endif