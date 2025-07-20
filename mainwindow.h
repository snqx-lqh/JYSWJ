#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSettings>
#include <QDir>
#include <QDebug>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void onUpdateConnectInfo(QString);

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::MainWindow *ui;

    QString   iniFile;
    QSettings *settings;
    QString ConnectInfo;
    QLabel *lb_ConnectInfo;
};
#endif // MAINWINDOW_H
