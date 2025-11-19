/*
 * IO设置文件，串口，UDP，TCP的配置相关就在这个文件中
 *
 */
#include "iosettingsform.h"
#include "ui_iosettingsform.h"

IOSettingsForm::IOSettingsForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::IOSettingsForm)
{
    ui->setupUi(this);

    QDir dir(QCoreApplication::applicationDirPath());
    mIniFile = dir.filePath("Config/settings.ini");

    ui->comboBox_PortName->setItemDelegate(new QStyledItemDelegate(ui->comboBox_PortName));

    mSerialIOService.scanAvailableSerialPort(ui->comboBox_PortName);
    ui->comboBox_PortName->addItem("TCPClient");
    ui->comboBox_PortName->addItem("TCPServer");
    ui->comboBox_PortName->addItem("UDP");
    mSerialIOService.addCommonBaudItem(ui->comboBox_BaudRate);
    QStringList dataBitsTemp;
    dataBitsTemp<<"5"<<"6"<<"7"<<"8";
    ui->comboBox_DataBits->addItems(dataBitsTemp);
    QStringList stopBitsTemp;
    stopBitsTemp<<"1"<<"1.5"<<"2";
    ui->comboBox_StopBits->addItems(stopBitsTemp);
    QStringList parityTemp;
    parityTemp<<"None"<<"Even"<<"Odd";
    ui->comboBox_Parity->addItems(parityTemp);

    connect(&mSerialIOService,&SerialIOService::readBytes,this,&IOSettingsForm::onReadBytes);
    connect(&mSerialIOService,&SerialIOService::sendBytesCount,this,&IOSettingsForm::onSendCountChanged);
    connect(&mSerialIOService,&SerialIOService::recvBytesCount,this,&IOSettingsForm::onRecvCountChanged);

    mUdpIOService.scanLocalAvlidAddr(ui->cmb_UdpLocalAddr);
    connect(&mUdpIOService,&UdpIOService::readBytes,this,&IOSettingsForm::onReadBytes);

    mTcpServerIOService.scanLocalAvlidAddr(ui->cmb_tcpServerLocalAddr);
    connect(&mTcpServerIOService,&TcpServerIOService::readBytes,this,&IOSettingsForm::onReadBytes);
    connect(&mTcpServerIOService,&TcpServerIOService::addOneConnect,this,[this](QString ip,QString port){
        ui->cmb_tcpServerAimAddr->addItem(ip+':'+port);
        tcpServerConnectNum++;
        ui->cmb_tcpServerAimAddr->setItemText(0,QString("All Connect(%1)").arg(tcpServerConnectNum));
    });
    connect(&mTcpServerIOService,&TcpServerIOService::delOneConnect,this,[this](QString ip,QString port){
        QString target = ip + ':' + port;
        // 遍历查找对应的行
        for (int i = 0; i < ui->cmb_tcpServerAimAddr->count(); ++i)
        {
            if (ui->cmb_tcpServerAimAddr->itemText(i) == target)
            {
                tcpServerConnectNum--;
                ui->cmb_tcpServerAimAddr->removeItem(i);
                ui->cmb_tcpServerAimAddr->setItemText(0,QString("All Connect(%1)").arg(tcpServerConnectNum));
                break;                 // 找到就删，跳出循环
            }
        }
    });

    mTcpClientIOService.scanLocalAvlidAddr(ui->cmb_tcpClientLocalAddr);
    connect(&mTcpClientIOService,&TcpClientIOService::readBytes,this,&IOSettingsForm::onReadBytes);
    connect(&mTcpClientIOService,&TcpClientIOService::getLocalPort,this,[this](quint16 port){
        ui->le_tcpClientLocalPort->setText(QString::number(port));
    });
    connect(&mTcpClientIOService,&TcpClientIOService::connectionResult,this,[](bool success, QString message){
        qDebug()<<"sucess:"<<success<<" message"<<message;
    });

    ui->pushButton_Setting->setVisible(false);

    loadSettings();

    qApp->installNativeEventFilter(this);
}

IOSettingsForm::~IOSettingsForm()
{
    saveSettings();
    delete ui;
}

void IOSettingsForm::loadSettings()
{
    QSettings settings(mIniFile,QSettings::IniFormat);
    settings.beginGroup("IOSettingsForm");

    auto PortName        = settings.value("PortName","");
    auto BaudRate        = settings.value("BaudRate","115200");
    auto DataBits        = settings.value("DataBits","8");
    auto StopBits        = settings.value("StopBits","1");
    auto Parity          = settings.value("Parity","None");
    auto SendNewLine     = settings.value("SendNewLine","false");
    auto SendShow        = settings.value("SendShow","false");
    auto HexShow         = settings.value("HexShow","false");
    auto DateTime        = settings.value("DateTime","false");

    auto cmb_tcpClientLocalAddr        = settings.value("cmb_tcpClientLocalAddr",ui->cmb_tcpClientLocalAddr->currentText());
    auto le_tcpClientAimAddr          = settings.value("le_tcpClientAimAddr",ui->le_tcpClientAimAddr->text());
    auto le_tcpClientAimPort          = settings.value("le_tcpClientAimPort",ui->le_tcpClientAimPort->text());

    auto le_tcpServerLocalPort        = settings.value("le_tcpServerLocalPort",ui->le_tcpServerLocalPort->text());
    auto cmb_tcpServerLocalAddr       = settings.value("cmb_tcpServerLocalAddr",ui->cmb_tcpServerLocalAddr->currentText());

    auto cmb_UdpLocalAddr          = settings.value("cmb_UdpLocalAddr",ui->cmb_UdpLocalAddr->currentText());
    auto le_UdpLocalPort        = settings.value("le_UdpLocalPort",ui->le_UdpLocalPort->text());
    auto le_UdpAimAddr          = settings.value("le_UdpAimAddr",ui->le_UdpAimAddr->text());
    auto le_UdpAimPort          = settings.value("le_UdpAimPort",ui->le_UdpAimPort->text());

    settings.endGroup();

    ui->comboBox_PortName->setCurrentText(PortName.toString());
    ui->comboBox_BaudRate->setCurrentText(BaudRate.toString());
    ui->comboBox_DataBits->setCurrentText(DataBits.toString());
    ui->comboBox_StopBits->setCurrentText(StopBits.toString());
    ui->comboBox_Parity->setCurrentText(Parity.toString());

    ui->cmb_tcpClientLocalAddr->setCurrentText(cmb_tcpClientLocalAddr.toString());
    ui->le_tcpClientAimAddr->setText(le_tcpClientAimAddr.toString());
    ui->le_tcpClientAimPort->setText(le_tcpClientAimPort.toString());
    ui->le_tcpServerLocalPort->setText(le_tcpServerLocalPort.toString());
    ui->cmb_tcpServerLocalAddr->setCurrentText(cmb_tcpServerLocalAddr.toString());
    ui->cmb_UdpLocalAddr->setCurrentText(cmb_UdpLocalAddr.toString());
    ui->le_UdpLocalPort->setText(le_UdpLocalPort.toString());
    ui->le_UdpAimAddr->setText(le_UdpAimAddr.toString());
    ui->le_UdpAimPort->setText(le_UdpAimPort.toString());

}

void IOSettingsForm::saveSettings()
{
    QSettings settings(mIniFile,QSettings::IniFormat);
    settings.beginGroup("IOSettingsForm");

    settings.setValue("PortName", ui->comboBox_PortName->currentText());
    settings.setValue("BaudRate", ui->comboBox_BaudRate->currentText());
    settings.setValue("DataBits", ui->comboBox_DataBits->currentText());
    settings.setValue("StopBits", ui->comboBox_StopBits->currentText());
    settings.setValue("Parity"  , ui->comboBox_Parity->currentText());

    settings.setValue("cmb_tcpClientLocalAddr",ui->cmb_tcpClientLocalAddr->currentText());
    settings.setValue("le_tcpClientAimAddr",ui->le_tcpClientAimAddr->text());
    settings.setValue("le_tcpClientAimPort",ui->le_tcpClientAimPort->text());

    settings.setValue("le_tcpServerLocalPort",ui->le_tcpServerLocalPort->text());
    settings.setValue("cmb_tcpServerLocalAddr",ui->cmb_tcpServerLocalAddr->currentText());

    settings.setValue("cmb_UdpLocalAddr",ui->cmb_UdpLocalAddr->currentText());
    settings.setValue("le_UdpLocalPort",ui->le_UdpLocalPort->text());
    settings.setValue("le_UdpAimAddr",ui->le_UdpAimAddr->text());
    settings.setValue("le_UdpAimPort",ui->le_UdpAimPort->text());

    settings.endGroup();
}

void IOSettingsForm::setProgressBar(QProgressBar *_progressBar)
{
    mSerialIOService.setProgressBar(_progressBar);
}

void IOSettingsForm::stateInit()
{
    if(connectMode == IOSettingsForm::TcpClient){
        emit connectInfo("TCP Client Mode");
    }else if(connectMode == IOSettingsForm::TcpServer){
        emit connectInfo("TCP Server Mode");
    }else if(connectMode == IOSettingsForm::Udp){
        emit connectInfo("UDP Mode");
    }else{
        emit connectInfo(mSerialIOService.getSerialConnectInfo());
    }
}

void IOSettingsForm::onReadBytes(QByteArray bytes)
{
    emit readBytes(bytes);
    if(receiveFile.isOpen()){
        receiveFile.write(bytes);
    }
}

void IOSettingsForm::onSendBytes(QByteArray bytes)
{
    if(connectMode == IOSettingsForm::TcpClient){
        mTcpClientIOService.sendBytes(bytes);
    }else if(connectMode == IOSettingsForm::TcpServer){
        mTcpServerIOService.sendBytes(ui->cmb_tcpServerAimAddr->currentText(),bytes);
    }else if(connectMode == IOSettingsForm::Udp){
        mUdpIOService.sendBytes(bytes);
    }else if(connectMode == IOSettingsForm::Serial) {
        mSerialIOService.sendBytes(bytes);
    }
    if(showSendStr){
        emit appendSendData(bytes);
    }
}

void IOSettingsForm::onProtocolSendBytes(QByteArray bytes)
{
    mSerialIOService.sendBytes(bytes);
}

void IOSettingsForm::onSendFile(QString filePath)
{
    mSerialIOService.sendFile(filePath);
}

void IOSettingsForm::onStateChange(STATE_CHANGE_TYPE_T type, int state)
{
    if(type == IOConnect_State){
        if(state == 0){
            bool stateTemp = false;
            if(connectMode == Serial){
                stateTemp = mSerialIOService.openSerial();
                if(stateTemp){
                    ui->comboBox_PortName->setEnabled(false);
                    ui->comboBox_BaudRate->setEnabled(false);
                    ui->comboBox_DataBits->setEnabled(false);
                    ui->comboBox_StopBits->setEnabled(false);
                    ui->comboBox_Parity->setEnabled(false);
                }
            }else if(connectMode == Udp){
                stateTemp = mUdpIOService.bindAimAddressAndPort(ui->le_UdpAimAddr->text(),ui->le_UdpAimPort->text(),
                                                    ui->cmb_UdpLocalAddr->currentText(),ui->le_UdpLocalPort->text());
                if(stateTemp){
                    ui->comboBox_PortName->setEnabled(false);
                    ui->le_UdpAimAddr->setEnabled(false);
                    ui->le_UdpAimPort->setEnabled(false);
                    ui->cmb_UdpLocalAddr->setEnabled(false);
                    ui->le_UdpLocalPort->setEnabled(false);
                }
            }else if(connectMode == TcpClient){
                stateTemp = mTcpClientIOService.connectServer(ui->le_tcpClientAimAddr->text(),ui->le_tcpClientAimPort->text().toUInt());
                if(stateTemp){
                    ui->comboBox_PortName->setEnabled(false);
                    ui->le_tcpClientAimAddr->setEnabled(false);
                    ui->le_tcpClientAimPort->setEnabled(false);
                    ui->cmb_tcpClientLocalAddr->setEnabled(false);
                }
            }else if(connectMode == TcpServer){
                stateTemp = mTcpServerIOService.setLocalAddrAndPort(ui->cmb_tcpServerLocalAddr->currentText(),ui->le_tcpServerLocalPort->text().toUInt());
                if(stateTemp){
                    ui->comboBox_PortName->setEnabled(false);
                    ui->cmb_tcpServerLocalAddr->setEnabled(false);
                    ui->le_tcpServerLocalPort->setEnabled(false);
                }
            }
            if(stateTemp == false){
                QMessageBox::warning(nullptr,"警告","通信IO连接失败");
            }
            emit stateChange(IOConnect_State,stateTemp);
        }else {
            if(connectMode == Serial){
                mSerialIOService.closeSerial();
                ui->comboBox_PortName->setEnabled(true);
                ui->comboBox_BaudRate->setEnabled(true);
                ui->comboBox_DataBits->setEnabled(true);
                ui->comboBox_StopBits->setEnabled(true);
                ui->comboBox_Parity->setEnabled(true);
            }else if(connectMode == Udp){
                mUdpIOService.udpClose();
                ui->comboBox_PortName->setEnabled(true);
                ui->le_UdpAimAddr->setEnabled(true);
                ui->le_UdpAimPort->setEnabled(true);
                ui->cmb_UdpLocalAddr->setEnabled(true);
                ui->le_UdpLocalPort->setEnabled(true);
            }else if(connectMode == TcpClient){
                mTcpClientIOService.disconnectServer();
                ui->comboBox_PortName->setEnabled(true);
                ui->le_tcpClientAimAddr->setEnabled(true);
                ui->le_tcpClientAimPort->setEnabled(true);
                ui->cmb_tcpClientLocalAddr->setEnabled(true);
            }else if(connectMode == TcpServer){
                mTcpServerIOService.stopServer();
                ui->comboBox_PortName->setEnabled(true);
                ui->cmb_tcpServerLocalAddr->setEnabled(true);
                ui->le_tcpServerLocalPort->setEnabled(true);
            }
            emit stateChange(IOConnect_State, 0);
        }
    }else if(type == RecvShowSend_State)
    {
        if(state == 0){
            showSendStr = false;
        }else{
            showSendStr = true;
        }
    }
}

void IOSettingsForm::on_comboBox_PortName_currentTextChanged(const QString &arg1)
{
    if(arg1 == "TCPClient"){
        ui->stackedWidget->setCurrentIndex(1);
        emit connectInfo("TCP Client Mode");
        connectMode = IOSettingsForm::TcpClient;
    }else if(arg1 == "TCPServer"){
        ui->stackedWidget->setCurrentIndex(2);
        emit connectInfo("TCP Server Mode");
        connectMode = IOSettingsForm::TcpServer;
    }else if(arg1 == "UDP"){
        ui->stackedWidget->setCurrentIndex(3);
        emit connectInfo("UDP Mode");
        connectMode = IOSettingsForm::Udp;
    }else{
        ui->stackedWidget->setCurrentIndex(0);
        mSerialIOService.setSerialPortName(arg1.split(" ").at(0));
        emit connectInfo(mSerialIOService.getSerialConnectInfo());
        connectMode = IOSettingsForm::Serial;
    }
}

void IOSettingsForm::on_comboBox_BaudRate_currentTextChanged(const QString &arg1)
{
    mSerialIOService.setSerialBaudRate(arg1);
    emit connectInfo(mSerialIOService.getSerialConnectInfo());
}

void IOSettingsForm::on_comboBox_StopBits_currentTextChanged(const QString &arg1)
{
    mSerialIOService.setSerialStopBits(arg1);
    emit connectInfo(mSerialIOService.getSerialConnectInfo());
}

void IOSettingsForm::on_comboBox_DataBits_currentTextChanged(const QString &arg1)
{
    mSerialIOService.setSerialDataBits(arg1);
    emit connectInfo(mSerialIOService.getSerialConnectInfo());
}

void IOSettingsForm::on_comboBox_Parity_currentTextChanged(const QString &arg1)
{
    mSerialIOService.setSerialParity(arg1);
    emit connectInfo(mSerialIOService.getSerialConnectInfo());
}

void IOSettingsForm::onSendCountChanged(quint32 count)
{
    emit stateChange(SendCount,count);
}

void IOSettingsForm::onRecvCountChanged(quint32 count)
{
    emit stateChange(RecvCount,count);
}

// 热插拔检测端口的代码
bool IOSettingsForm::nativeEventFilter(const QByteArray & eventType, void * message, long * result)
{
    MSG* msg = reinterpret_cast<MSG*>(message);
    int msgType = msg->message;
    if (msgType == WM_DEVICECHANGE) {
        PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)msg->lParam;
        switch (msg->wParam) {
        case DBT_DEVICEARRIVAL:
            if (lpdb->dbch_devicetype == DBT_DEVTYP_PORT) {
                PDEV_BROADCAST_PORT lpdbv = (PDEV_BROADCAST_PORT)lpdb;
                QString port = QString::fromWCharArray(lpdbv->dbcp_name);//插入的串口名
                QString com  = ui->comboBox_PortName->currentText();
                bool serialConnect = false;
                serialConnect = mSerialIOService.isSerialOpen();
                if(serialConnect) mSerialIOService.closeSerial();
                mSerialIOService.scanAvailableSerialPort(ui->comboBox_PortName);
                if(serialConnect) mSerialIOService.openSerial();
                ui->comboBox_PortName->addItem("TCPClient");
                ui->comboBox_PortName->addItem("TCPServer");
                ui->comboBox_PortName->addItem("UDP");
                ui->comboBox_PortName->setCurrentText(com);
            }
            break;
        case DBT_DEVICEREMOVECOMPLETE:
            if (lpdb->dbch_devicetype == DBT_DEVTYP_PORT) {
                PDEV_BROADCAST_PORT lpdbv = (PDEV_BROADCAST_PORT)lpdb;
                QString port = QString::fromWCharArray(lpdbv->dbcp_name);//拔出的串口名

                QString com  = "";
                if(connectMode == Serial){
                    com  = ui->comboBox_PortName->currentText().split(" ").at(0);
                    bool serialConnect = false;
                    serialConnect = mSerialIOService.isSerialOpen();
                    emit stateChange(IOConnect_State, 0);
                    if(serialConnect) mSerialIOService.closeSerial();
                    ui->comboBox_PortName->setEnabled(true);
                    ui->comboBox_BaudRate->setEnabled(true);
                    ui->comboBox_DataBits->setEnabled(true);
                    ui->comboBox_StopBits->setEnabled(true);
                    ui->comboBox_Parity->setEnabled(true);
                }else{
                    com  = ui->comboBox_PortName->currentText();
                }
                mSerialIOService.scanAvailableSerialPort(ui->comboBox_PortName);
                ui->comboBox_PortName->addItem("TCPClient");
                ui->comboBox_PortName->addItem("TCPServer");
                ui->comboBox_PortName->addItem("UDP");
                ui->comboBox_PortName->setCurrentText(com);
            }
            break;
        case DBT_DEVNODES_CHANGED:
            break;
        default:
            break;
        }
    }
    return QWidget::nativeEvent(eventType, message, result);
}

