#include "SplashScreen.h"

#include <QBoxLayout>

SplashScreen::SplashScreen(QWidget* parent)
    : QDialog(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout;

    setLayout(mainLayout);
}

SplashScreen::~SplashScreen()
{
}

