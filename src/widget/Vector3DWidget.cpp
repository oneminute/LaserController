#include "Vector3DWidget.h"

#include <QBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>

class Vector3DWidgetPrivate
{
    Q_DECLARE_PUBLIC(Vector3DWidget)
public:
    Vector3DWidgetPrivate(Vector3DWidget* ptr, qreal _x, qreal _y, qreal _z)
        : q_ptr(ptr)
        , x(_x)
        , y(_y)
        , z(_z)
    {}

    Vector3DWidget* q_ptr;
    qreal x;
    qreal y;
    qreal z;
    QDoubleSpinBox* spinBoxX;
    QDoubleSpinBox* spinBoxY;
    QDoubleSpinBox* spinBoxZ;
    QLabel* labelX;
    QLabel* labelY;
    QLabel* labelZ;
};

Vector3DWidget::Vector3DWidget(QWidget* parent)
    : QWidget(parent)
    , d_ptr(new Vector3DWidgetPrivate(this, 0, 0, 0))
{
    init();
}

Vector3DWidget::Vector3DWidget(qreal x, qreal y, qreal z, QWidget* parent)
    : QWidget(parent)
    , d_ptr(new Vector3DWidgetPrivate(this, x, y, z))
{
    init();
}

Vector3DWidget::Vector3DWidget(const QVector3D& v, QWidget* parent)
    : QWidget(parent)
    , d_ptr(new Vector3DWidgetPrivate(this, v.x(), v.y(), v.z()))
{
    init();
}

Vector3DWidget::~Vector3DWidget()
{
}

qreal Vector3DWidget::x() const
{
    Q_D(const Vector3DWidget);
    return d->x;
}

void Vector3DWidget::setX(qreal x)
{
    Q_D(Vector3DWidget);
    d->x = x;
    d->spinBoxX->setValue(x);
    emit valueChanged(d->x, d->y, d->z);
}

qreal Vector3DWidget::y() const
{
    Q_D(const Vector3DWidget);
    return d->y;
}

void Vector3DWidget::setY(qreal y)
{
    Q_D(Vector3DWidget);
    d->y = y;
    d->spinBoxY->setValue(y);
    emit valueChanged(d->x, d->y, d->z);
}

qreal Vector3DWidget::z() const
{
    Q_D(const Vector3DWidget);
    return d->z;
}

void Vector3DWidget::setZ(qreal z)
{
    Q_D(Vector3DWidget);
    d->z = z;
    d->spinBoxZ->setValue(z);
    emit valueChanged(d->x, d->y, d->z);
}

void Vector3DWidget::setValue(qreal x, qreal y, qreal z)
{
    Q_D(Vector3DWidget);
    d->x = x;
    d->y = y;
    d->z = z;
    d->spinBoxX->setValue(x);
    d->spinBoxY->setValue(y);
    d->spinBoxZ->setValue(z);
    emit valueChanged(d->x, d->y, d->z);
}

void Vector3DWidget::setValue(const QVector3D& v)
{
    setValue(v.x(), v.y(), v.z());
}

QVector3D Vector3DWidget::toVector3D() const
{
    Q_D(const Vector3DWidget);
    return QVector3D(d->x, d->y, d->z);
}

qreal Vector3DWidget::minimum() const
{
    Q_D(const Vector3DWidget);
    return d->spinBoxX->minimum();
}

qreal Vector3DWidget::maximum() const
{
    Q_D(const Vector3DWidget);
    return d->spinBoxX->maximum();
}

qreal Vector3DWidget::xMinimum() const
{
    Q_D(const Vector3DWidget);
    return d->spinBoxX->minimum();
}

qreal Vector3DWidget::yMinimum() const
{
    Q_D(const Vector3DWidget);
    return d->spinBoxY->minimum();
}

qreal Vector3DWidget::zMinimum() const
{
    Q_D(const Vector3DWidget);
    return d->spinBoxZ->minimum();
}

qreal Vector3DWidget::xMaximum() const
{
    Q_D(const Vector3DWidget);
    return d->spinBoxX->maximum();
}

qreal Vector3DWidget::yMaximum() const
{
    Q_D(const Vector3DWidget);
    return d->spinBoxY->maximum();
}

qreal Vector3DWidget::zMaximum() const
{
    Q_D(const Vector3DWidget);
    return d->spinBoxZ->maximum();
}

void Vector3DWidget::setMinimum(qreal value)
{
    Q_D(Vector3DWidget);
    d->spinBoxX->setMinimum(value);
    d->spinBoxY->setMinimum(value);
    d->spinBoxZ->setMinimum(value);
}

void Vector3DWidget::setMaximum(qreal value)
{
    Q_D(Vector3DWidget);
    d->spinBoxX->setMaximum(value);
    d->spinBoxY->setMaximum(value);
    d->spinBoxZ->setMaximum(value);
}

void Vector3DWidget::setXMinimum(qreal value)
{
    Q_D(Vector3DWidget);
    d->spinBoxX->setMinimum(value);
}

void Vector3DWidget::setYMinimum(qreal value)
{
    Q_D(Vector3DWidget);
    d->spinBoxY->setMinimum(value);
}

void Vector3DWidget::setZMinimum(qreal value)
{
    Q_D(Vector3DWidget);
    d->spinBoxZ->setMinimum(value);
}

void Vector3DWidget::setXMaximum(qreal value)
{
    Q_D(Vector3DWidget);
    d->spinBoxX->setMaximum(value);
}

void Vector3DWidget::setYMaximum(qreal value)
{
    Q_D(Vector3DWidget);
    d->spinBoxY->setMaximum(value);
}

void Vector3DWidget::setZMaximum(qreal value)
{
    Q_D(Vector3DWidget);
    d->spinBoxZ->setMaximum(value);
}

int Vector3DWidget::decimals() const
{
    Q_D(const Vector3DWidget);
    return d->spinBoxX->decimals();
}

void Vector3DWidget::setDecimals(int value)
{
    Q_D(Vector3DWidget);
    d->spinBoxX->setDecimals(value);
    d->spinBoxY->setDecimals(value);
    d->spinBoxZ->setDecimals(value);
}

QString Vector3DWidget::xTitle() const
{
    Q_D(const Vector3DWidget);
    return d->labelX->text();
}

void Vector3DWidget::setXTitle(const QString& title)
{
    Q_D(Vector3DWidget);
    d->labelX->setText(title);
}

QString Vector3DWidget::yTitle() const
{
    Q_D(const Vector3DWidget);
    return d->labelY->text();
}

void Vector3DWidget::setYTitle(const QString& title)
{
    Q_D(Vector3DWidget);
    d->labelY->setText(title);
}

QString Vector3DWidget::zTitle() const
{
    Q_D(const Vector3DWidget);
    return d->labelZ->text();
}

void Vector3DWidget::setZTitle(const QString& title)
{
    Q_D(Vector3DWidget);
    d->labelZ->setText(title);
}

void Vector3DWidget::init()
{
    Q_D(Vector3DWidget);

    d->labelX = new QLabel(tr(" X:"), this);
    d->labelY = new QLabel(tr(" Y:"), this);
    d->labelZ = new QLabel(tr(" Z:"), this);
    d->spinBoxX = new QDoubleSpinBox(this);
    d->spinBoxY = new QDoubleSpinBox(this);
    d->spinBoxZ = new QDoubleSpinBox(this);

    QHBoxLayout* layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addWidget(d->labelX);
    layout->addWidget(d->spinBoxX);
    layout->addWidget(d->labelY);
    layout->addWidget(d->spinBoxY);
    layout->addWidget(d->labelZ);
    layout->addWidget(d->spinBoxZ);
    layout->setStretch(0, 0);
    layout->setStretch(1, 1);
    layout->setStretch(2, 0);
    layout->setStretch(3, 1);
    layout->setStretch(4, 0);
    layout->setStretch(5, 1);

    this->setLayout(layout);

    connect(d->spinBoxX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Vector3DWidget::onComboBoxXValueChanged);
    connect(d->spinBoxY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Vector3DWidget::onComboBoxYValueChanged);
    connect(d->spinBoxZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Vector3DWidget::onComboBoxZValueChanged);
}

void Vector3DWidget::onComboBoxXValueChanged(qreal value)
{
    setX(value);
}

void Vector3DWidget::onComboBoxYValueChanged(qreal value)
{
    setY(value);
}

void Vector3DWidget::onComboBoxZValueChanged(qreal value)
{
    setZ(value);
}
