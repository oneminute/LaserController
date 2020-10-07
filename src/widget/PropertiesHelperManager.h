#ifndef PROPERTIESHELPERMANAGER_H
#define PROPERTIESHELPERMANAGER_H

#include <QObject>
#include "widget/LaserPrimitivePropertiesHelper.h"

class PropertiesHelperManager
{
public:
    static LaserPrimitivePropertiesHelper& primitivePropertiesHelper();
};

#endif // PROPERTIESHELPERMANAGER_H