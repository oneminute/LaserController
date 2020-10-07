#ifndef LASERPRIMITIVEPROPERTIESHELPER
#define LASERPRIMITIVEPROPERTIESHELPER

#include <QObject>

class QTableWidget;
class LaserPrimitive;

class LaserPrimitivePropertiesHelper
{
public:
    LaserPrimitivePropertiesHelper();

    void resetProperties(LaserPrimitive* primitive, QTableWidget* table);

protected:

private:
    QTableWidget* m_table;
    LaserPrimitive* m_primitive;
};

#endif // LASERPRIMITIVEPROPERTIESHELPER
