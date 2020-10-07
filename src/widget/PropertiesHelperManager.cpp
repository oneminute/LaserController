#include "PropertiesHelperManager.h"

LaserPrimitivePropertiesHelper & PropertiesHelperManager::primitivePropertiesHelper()
{
    static LaserPrimitivePropertiesHelper helper;
    return helper;
}
