#ifndef LAYERBUTTON_H
#define LAYERBUTTON_H

#include "common/common.h"
#include "widget/LaserViewer.h"
#include <QLabel>
#include <QWidget>
#include <QColor>

class LaserLayer;

class LayerButton : public QLabel
{
    Q_OBJECT
public:
    explicit LayerButton(LaserViewer* viewer, QWidget* parent = nullptr);
    virtual ~LayerButton();

    QColor color() const;
    void setColor(const QColor& color);

    LaserLayer* layer() const { return m_layer; }
    void setLayer(LaserLayer* layer) { m_layer = layer; }
	bool checked();
	void setChecked(bool checked);
	void setLayerIndex(int index);
	void setCheckedTrue();

protected:
    virtual void contextMenuEvent(QContextMenuEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual bool eventFilter(QObject *watched, QEvent *event) override;
    virtual void paintEvent(QPaintEvent* event) override;

signals:
    void clicked();
    void colorUpdated();

private:
    QColor m_color;
    bool m_pressed;
	//bool m_clicked;
	bool m_moved;
	bool m_checked;
    LaserLayer* m_layer;
	LaserViewer* m_viewer;
	int m_layerIndex;

};

#endif // LAYERBUTTON_H