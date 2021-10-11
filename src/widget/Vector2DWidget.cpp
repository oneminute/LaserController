#include "Vector2DWidget.h"

#include <QBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>

class Vector2DWidgetPrivate
{
    Q_DECLARE_PUBLIC(Vector2DWidget)
public:
    Vector2DWidgetPrivate(Vector2DWidget* ptr, qreal _x, qreal _y)
        : q_ptr(ptr)
        , x(_x)
        , y(_y)
    {}

    Vector2DWidget* q_ptr;
    qreal x;
    qreal y;
    QDoubleSpinBox* spinBoxX;
    QDoubleSpinBox* spinBoxY;
};

Vector2DWidget::Vector2DWidget(QWidget* parent)
    : QWidget(parent)
    , d_ptr(new Vector2DWidgetPrivate(this, 0, 0))
{
    init();
}

Vector2DWidget::Vector2DWidget(qreal x, qreal y, QWidget* parent)
    : QWidget(parent)
    , d_ptr(new Vector2DWidgetPrivate(this, x, y))
{
    init();
}

Vector2DWidget::Vector2DWidget(const QVector2D& v, QWidget* parent)
    : QWidget(parent)
    , d_ptr(new Vector2DWidgetPrivate(this, v.x(), v.y()))
{
    init();
}

Vector2DWidget::Vector2DWidget(const QPointF& pt, QWidget* parent)
    : QWidget(parent)
    , d_ptr(new Vector2DWidgetPrivate(this, pt.x(), pt.y()))
{
    init();
}

Vector2DWidget::Vector2DWidget(const QPoint& pt, QWidget* parent)
    : QWidget(parent)
    , d_ptr(new Vector2DWidgetPrivate(this, pt.x(), pt.y()))
{
    init();
}

Vector2DWidget::~Vector2DWidget()
{
}

qreal Vector2DWidget::x() const
{
    Q_D(const Vector2DWidget);
    return d->x;
}

void Vector2DWidget::setX(qreal x)
{
    Q_D(Vector2DWidget);
    d->x = x;
    d->spinBoxX->setValue(x);
    emit valueChanged(d->x, d->y);
}

qreal Vector2DWidget::y() const
{
    Q_D(const Vector2DWidget);
    return d->y;
}

void Vector2DWidget::setY(qreal y)
{
    Q_D(Vector2DWidget);
    d->y = y;
    d->spinBoxY->setValue(y);
    emit valueChanged(d->x, d->y);
}

void Vector2DWidget::setXY(qreal x, qreal y)
{
    Q_D(Vector2DWidget);
    d->x = x;
    d->y = y;
    d->spinBoxX->setValue(x);
    d->spinBoxY->setValue(y);
    emit valueChanged(d->x, d->y);
}

void Vector2DWidget::setValue(const QPointF& pt)
{
    setXY(pt.x(), pt.y());
}

void Vector2DWidget::setValue(const QPoint& pt)
{
    setXY(pt.x(), pt.y());
}

void Vector2DWidget::setValue(const QVector2D& v)
{
    setXY(v.x(), v.y());
}

QVector2D Vector2DWidget::toVector2D() const
{
    Q_D(const Vector2DWidget);
    return QVector2D(d->x, d->y);
}

QPointF Vector2DWidget::toPointF() const
{
    Q_D(const Vector2DWidget);
    return QPointF(d->x, d->y);
}

QPoint Vector2DWidget::toPoint() const
{
    Q_D(const Vector2DWidget);
    return QPoint(qRound(d->x), qRound(d->y));
}

void Vector2DWidget::init()
{
    Q_D(Vector2DWidget);

    QLabel* labelX = new QLabel(tr(" X:"), this);
    QLabel* labelY = new QLabel(tr(" Y:"), this);
    d->spinBoxX = new QDoubleSpinBox(this);
    d->spinBoxY = new QDoubleSpinBox(this);

    QHBoxLayout* layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addWidget(labelX);
    layout->addWidget(d->spinBoxX);
    layout->addWidget(labelY);
    layout->addWidget(d->spinBoxY);
    layout->setStretch(0, 0);
    layout->setStretch(1, 1);
    layout->setStretch(2, 0);
    layout->setStretch(3, 1);

    this->setLayout(layout);

    connect(d->spinBoxX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Vector2DWidget::onComboBoxXValueChanged);
    connect(d->spinBoxY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Vector2DWidget::onComboBoxYValueChanged);
}

void Vector2DWidget::onComboBoxXValueChanged(qreal value)
{
    setX(value);
}

void Vector2DWidget::onComboBoxYValueChanged(qreal value)
{
    setY(value);
}
