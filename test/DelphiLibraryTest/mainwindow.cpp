#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QWidget>
#include <QWindow>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QWindow* window = QWindow::fromWinId(m_library.showForm1());
    QWidget* externalWidget = createWindowContainer(window);
    ui->verticalLayout_2->addWidget(externalWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButtonAddTwoStd_clicked()
{
    m_library.addTwo();
}

