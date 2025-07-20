#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("简易上位机" + QString(APPVERSION));

    QDir dir(QCoreApplication::applicationDirPath());
    QString iniFile = dir.filePath("settings.ini");

    settings = new QSettings(iniFile,QSettings::IniFormat);

    ui->groupBox_sendMul->setVisible(false);

    lb_ConnectInfo = new QLabel(tr("COM,110,8,1,None,None"));
    ui->statusbar->addWidget(lb_ConnectInfo);
    QString connectInfo = ui->widget_portSet->getConnectInfo();
    lb_ConnectInfo->setText(connectInfo);

    connect(ui->widget_portSet,&PortSettingsForm::sendConnectInfo,this,&MainWindow::onUpdateConnectInfo);

    connect(ui->widget_portSet,&PortSettingsForm::readBytes,ui->widget_recv,&RecvAreaForm::onReadBytes);
    connect(ui->widget_portSet,&PortSettingsForm::clearRecv,ui->widget_recv,&RecvAreaForm::onClearRecv);
    connect(ui->widget_portSet,&PortSettingsForm::sendCheckTime,ui->widget_recv,&RecvAreaForm::onChangeAddTimeFlag);
    connect(ui->widget_portSet,&PortSettingsForm::sendHexShow,ui->widget_recv,&RecvAreaForm::onChangeHexShowFlag);
    connect(ui->widget_portSet,&PortSettingsForm::sendShowSend,ui->widget_recv,&RecvAreaForm::onChangeSendShowFlag);
    connect(ui->widget_portSet,&PortSettingsForm::sendSendHex,ui->widget_recv,&RecvAreaForm::onChangeSendHexFlag);
    connect(ui->widget_portSet,&PortSettingsForm::saveRecvEditData,ui->widget_recv,&RecvAreaForm::onSaveRecvEditData);

    connect(ui->widget_portSet,&PortSettingsForm::sendSendHex,ui->widget_send,&SendAreaForm::onChangeSendHexFlag);
    connect(ui->widget_send,&SendAreaForm::sendBytes,ui->widget_portSet,&PortSettingsForm::onSendBytes);
    connect(ui->widget_send,&SendAreaForm::sendBytes,ui->widget_recv,&RecvAreaForm::onAppendSendBytes);
    connect(ui->widget_send,&SendAreaForm::sendFile,ui->widget_portSet,&PortSettingsForm::onSendFile);
    connect(ui->widget_portSet,&PortSettingsForm::clearSend,ui->widget_send,&SendAreaForm::onClearSend);

    ui->widget_portSet->loadOptions(settings);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onUpdateConnectInfo(QString connectInfo)
{
    lb_ConnectInfo->setText(connectInfo);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    ui->widget_portSet->saveOptions(settings);
}

