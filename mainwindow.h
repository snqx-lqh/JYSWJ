#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QListWidget>
#include "common.h"
#include "versionintroductionform.h"
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void loadSettings();
    void saveSettings();

public slots:
    void onStateChange(STATE_CHANGE_TYPE_T type,int state);
private slots:


    void on_actionIntro_triggered();

private:
    Ui::MainWindow *ui;
    QString         mIniFile;
    VersionIntroductionForm mVersionIntroductionForm;
};
#endif // MAINWINDOW_H
