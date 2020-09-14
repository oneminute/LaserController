#ifndef LAYERBUTTON_H
#define LAYERBUTTON_H

#include "common/common.h"
#include <QLabel>
#include <QWidget>
#include <QColor>

class LaserLayer;

class LayerButton : public QLabel
{
    Q_OBJECT
public:
    explicit LayerButton(QWidget* parent = nullptr);
    virtual ~LayerButton();

    QColor color() const;
    void setColor(const QColor& color);

    LaserLayer* layer() const { return m_layer; }
    void setLayer(LaserLayer* layer) { m_layer = layer; }

protected:
    virtual void contextMenuEvent(QContextMenuEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void paintEvent(QPaintEvent* event) override;

signals:
    void clicked();
    void colorUpdated();

private:
    QColor m_color;
    bool m_pressed;
    LaserLayer* m_layer;
};

#endif // LAYERBUTTON_H