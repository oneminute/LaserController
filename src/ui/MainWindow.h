#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class SvgView;
class AboutDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool openFile(const QString& filename);
    bool openLibrary(const QString& dllname);

protected slots:
    void onActionFileOpen(bool checked = false);

private:
    Ui::MainWindow *ui;
    SvgView* m_svgView;
    QScopedPointer<AboutDialog> m_aboutDialog;

    QString m_currentPath;
};
#endif // MAINWINDOW_H
