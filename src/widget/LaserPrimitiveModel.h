#ifndef LASERPRIMITIVEMODEL_H
#define LASERPRIMITIVEMODEL_H

#include <QAbstractItemModel>

class LaserPrimitive;

class LaserPrimitiveModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit LaserPrimitiveModel(QObject* parent = nullptr);
    virtual ~LaserPrimitiveModel();

    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

    LaserPrimitive* primitive() const;
    void setPrimitive(LaserPrimitive* primitive);

private:
    LaserPrimitive* m_primitive;
};

#endif // LASERPRIMITIVEMODEL_H