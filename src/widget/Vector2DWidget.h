#ifndef VECTOR2DWIDGET_H
#define VECTOR2DWIDGET_H

#include <QWidget>
#include <QVector2D>
#include <QPointF>

class Vector2DWidgetPrivate;

class Vector2DWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal minimum READ minimum WRITE setMinimum DESIGNABLE true)
    Q_PROPERTY(qreal maximum READ maximum WRITE setMaximum DESIGNABLE true)
    Q_PROPERTY(qreal xMinimum READ xMinimum WRITE setXMinimum DESIGNABLE true)
    Q_PROPERTY(qreal xMaximum READ xMaximum WRITE setXMaximum DESIGNABLE true)
    Q_PROPERTY(qreal yMinimum READ yMinimum WRITE setYMinimum DESIGNABLE true)
    Q_PROPERTY(qreal yMaximum READ yMaximum WRITE setYMaximum DESIGNABLE true)
    Q_PROPERTY(int decimals READ decimals WRITE setDecimals DESIGNABLE true)
public:
    explicit Vector2DWidget(QWidget* parent = nullptr);
    explicit Vector2DWidget(qreal x, qreal y, QWidget* parent = nullptr);
    explicit Vector2DWidget(const QVector2D& v, QWidget* parent = nullptr);
    explicit Vector2DWidget(const QPointF& pt, QWidget* parent = nullptr);
    explicit Vector2DWidget(const QPoint& pt, QWidget* parent = nullptr);
    ~Vector2DWidget();

    qreal x() const;
    void setX(qreal x);

    qreal y() const;
    void setY(qreal y);

    void setXY(qreal x, qreal y);
    void setValue(const QPointF& pt);
    void setValue(const QPoint& pt);
    void setValue(const QVector2D& v);

    QVector2D toVector2D() const;
    QPointF toPointF() const;
    QPoint toPoint() const;

    qreal minimum() const;
    qreal maximum() const;
    qreal xMinimum() const;
    qreal yMinimum() const;
    qreal xMaximum() const;
    qreal yMaximum() const;
    void setMinimum(qreal value);
    void setMaximum(qreal value);
    void setXMinimum(qreal value);
    void setYMinimum(qreal value);
    void setXMaximum(qreal value);
    void setYMaximum(qreal value);

    int decimals() const;
    void setDecimals(int value);

protected:
    void init();

protected slots:
    void onComboBoxXValueChanged(qreal value);
    void onComboBoxYValueChanged(qreal value);

signals:
    void valueChanged(qreal x, qreal y);

private:
    QScopedPointer<Vector2DWidgetPrivate> d_ptr;

    Q_DECLARE_PRIVATE(Vector2DWidget)
    Q_DISABLE_COPY(Vector2DWidget)
};

#endif // VECTOR2DWIDGET_H