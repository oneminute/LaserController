#include "src/exception/LaserException.h"

#include <QDebug>
#include <QCoreApplication>

void test1() throw(LaserException)
{
    throw new LaserException(0, "");
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    try {
        test1();
    }
    catch (LaserException* e)
    {
        qDebug() << e;
    }

    return 0;
}