#ifndef VECTOR3DWIDGET_H
#define VECTOR3DWIDGET_H

#include <QWidget>
#include <QVector3D>
#include <QPointF>

class Vector3DWidgetPrivate;

class Vector3DWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal minimum READ minimum WRITE setMinimum DESIGNABLE true)
    Q_PROPERTY(qreal maximum READ maximum WRITE setMaximum DESIGNABLE true)
    Q_PROPERTY(qreal xMinimum READ xMinimum WRITE setXMinimum DESIGNABLE true)
    Q_PROPERTY(qreal xMaximum READ xMaximum WRITE setXMaximum DESIGNABLE true)
    Q_PROPERTY(qreal yMinimum READ yMinimum WRITE setYMinimum DESIGNABLE true)
    Q_PROPERTY(qreal yMaximum READ yMaximum WRITE setYMaximum DESIGNABLE true)
    Q_PROPERTY(qreal zMinimum READ zMinimum WRITE setZMinimum DESIGNABLE true)
    Q_PROPERTY(qreal zMaximum READ zMaximum WRITE setZMaximum DESIGNABLE true)
    Q_PROPERTY(QString xTitle READ xTitle WRITE setXTitle DESIGNABLE true)
    Q_PROPERTY(QString yTitle READ yTitle WRITE setYTitle DESIGNABLE true)
    Q_PROPERTY(QString zTitle READ zTitle WRITE setZTitle DESIGNABLE true)
    Q_PROPERTY(int decimals READ decimals WRITE setDecimals DESIGNABLE true)
public:
    explicit Vector3DWidget(QWidget* parent = nullptr);
    explicit Vector3DWidget(qreal x, qreal y, qreal z, QWidget* parent = nullptr);
    explicit Vector3DWidget(const QVector3D& v, QWidget* parent = nullptr);
    ~Vector3DWidget();

    qreal x() const;
    void setX(qreal x);

    qreal y() const;
    void setY(qreal y);

    qreal z() const;
    void setZ(qreal z);

    void setValue(qreal x, qreal y, qreal z);
    void setValue(const QVector3D& v);

    QVector3D toVector3D() const;

    qreal minimum() const;
    qreal maximum() const;
    qreal xMinimum() const;
    qreal yMinimum() const;
    qreal zMinimum() const;
    qreal xMaximum() const;
    qreal yMaximum() const;
    qreal zMaximum() const;
    void setMinimum(qreal value);
    void setMaximum(qreal value);
    void setXMinimum(qreal value);
    void setYMinimum(qreal value);
    void setZMinimum(qreal value);
    void setXMaximum(qreal value);
    void setYMaximum(qreal value);
    void setZMaximum(qreal value);

    int decimals() const;
    void setDecimals(int value);

    QString xTitle() const;
    void setXTitle(const QString& title);
    QString yTitle() const;
    void setYTitle(const QString& title);
    QString zTitle() const;
    void setZTitle(const QString& title);

protected:
    void init();

protected slots:
    void onComboBoxXValueChanged(qreal value);
    void onComboBoxYValueChanged(qreal value);
    void onComboBoxZValueChanged(qreal value);

signals:
    void valueChanged(qreal x, qreal y, qreal z);

private:
    QScopedPointer<Vector3DWidgetPrivate> d_ptr;

    Q_DECLARE_PRIVATE(Vector3DWidget)
    Q_DISABLE_COPY(Vector3DWidget)
};

#endif // VECTOR3DWIDGET_H