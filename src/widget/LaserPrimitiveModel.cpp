#include "LaserPrimitiveModel.h"

LaserPrimitiveModel::LaserPrimitiveModel(QObject* parent)
    : QAbstractItemModel(parent)
    , m_primitive(nullptr)
{

}

LaserPrimitiveModel::~LaserPrimitiveModel()
{
}

int LaserPrimitiveModel::columnCount(const QModelIndex & parent) const
{
    return 2;
}

LaserPrimitive * LaserPrimitiveModel::primitive() const
{
    return m_primitive;
}

void LaserPrimitiveModel::setPrimitive(LaserPrimitive * primitive)
{
    m_primitive = primitive;
}
