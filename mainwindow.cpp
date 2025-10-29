#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("简易上位机 V2.0.2");

    QDir dir(QCoreApplication::applicationDirPath());
    mIniFile = dir.filePath("Config/settings.ini");

    ui->splitter_2->setSizes({6500,3500});
//    ui->splitter_2->setStretchFactor(0, 1);
//    ui->splitter_2->setStretchFactor(1, 2);
    ui->splitter->setSizes({7000,3000});

    connect(ui->widget_ToolBar,&ToolBarForm::stateChange,this,&MainWindow::onStateChange);
    connect(ui->widget_ToolBar,&ToolBarForm::stateChange,ui->widget_IOSetting,&IOSettingsForm::onStateChange);
    connect(ui->widget_IOSetting,&IOSettingsForm::stateChange,ui->widget_ToolBar,&ToolBarForm::onStateChange);
    connect(ui->widget_IOSetting,&IOSettingsForm::stateChange,ui->widget_Send,&SendAreaForm::onStateChange);
    connect(ui->widget_IOSetting,&IOSettingsForm::stateChange,ui->widget_Recv,&RecvAreaForm::onStateChange);
    connect(ui->widget_IOSetting,&IOSettingsForm::stateChange,ui->widget_MultiSend,&MultiSendForm::onStateChange);
    connect(ui->widget_IOSetting,&IOSettingsForm::stateChange,ui->widget_state,&StateForm::onStateChange);

    connect(ui->widget_IOSetting,&IOSettingsForm::connectInfo,ui->widget_state,&StateForm::onConnectInfo);
    connect(ui->widget_IOSetting,&IOSettingsForm::readBytes,ui->widget_Recv,&RecvAreaForm::onReadBytes);
    connect(ui->widget_IOSetting,&IOSettingsForm::readBytes,ui->widget_Send,&SendAreaForm::onReadBytes);
    connect(ui->widget_IOSetting,&IOSettingsForm::appendSendData,ui->widget_Recv,&RecvAreaForm::onAppendSendData);
    connect(ui->widget_Recv,&RecvAreaForm::sendBytes,ui->widget_IOSetting,&IOSettingsForm::onSendBytes);
    connect(ui->widget_Send,&SendAreaForm::sendBytes,ui->widget_IOSetting,&IOSettingsForm::onSendBytes);
    connect(ui->widget_Send,&SendAreaForm::sendFile,ui->widget_IOSetting,&IOSettingsForm::onSendFile);

    ui->widget_IOSetting->setProgressBar(ui->widget_Send->getProgressBar());

    connect(ui->widget_MultiSend,&MultiSendForm::sendBytes,ui->widget_IOSetting,&IOSettingsForm::onSendBytes);

    ui->widget_IOSetting->stateInit();
    ui->widget_ToolBar->stateInit();

    ui->widget_ToolBar->setMinimumWidth(38);
    ui->widget_ToolBar->setMaximumWidth(38);
    ui->widget_IOSetting->setMinimumWidth(210);
    ui->widget_IOSetting->setMaximumWidth(210);

    loadSettings();
}

MainWindow::~MainWindow()
{
    saveSettings();
    delete ui;
}

void MainWindow::loadSettings()
{
    QSettings settings(mIniFile,QSettings::IniFormat);
    settings.beginGroup("MainWindow");

    auto widthTemp           = settings.value("widthTemp","1200");
    auto heightTemp          = settings.value("heightTemp","700");

    settings.endGroup();
    qDebug()<<QString("size %1 %2").arg(widthTemp.toInt()).arg(heightTemp.toInt());
    resize(QSize(widthTemp.toInt(), heightTemp.toInt()));
}

void MainWindow::saveSettings()
{
    QSettings settings(mIniFile,QSettings::IniFormat);
    settings.beginGroup("MainWindow");

    settings.setValue("widthTemp", this->size().width());
    settings.setValue("heightTemp", this->size().height());
    qDebug()<<QString("size %1 %2").arg(this->size().width()).arg(this->size().height());
    settings.endGroup();
}

void MainWindow::onStateChange(STATE_CHANGE_TYPE_T type, int state)
{
    if(type == IOSETTING_VISIBLE){
        bool stateTemp = (state==0?false:true);
        ui->widget_IOSetting->setVisible(stateTemp);
    }else if(type == MULTISEND_VISIBLE){
        bool stateTemp = (state==0?false:true);
        ui->widget_MultiSend->setVisible(stateTemp);
    }
}


void MainWindow::on_actionIntro_triggered()
{
    mVersionIntroductionForm.show();
}

