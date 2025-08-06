#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 标题，并带上版本号
    setWindowTitle("简易上位机" + QString(APPVERSION));

    // 找到配置文件，没有会自动创建
    QDir dir(QCoreApplication::applicationDirPath());
    QString iniFile = dir.filePath("settings.ini");
    settings = new QSettings(iniFile,QSettings::IniFormat);

    // 发送接收字符计数
    lb_SendRecvInfo= new QLabel(tr(""));
    lb_SendRecvInfo->setText(QString("发送：%1 接收：%2").arg(sendCount, 8).arg(recvCount, 8));
    ui->statusbar->addWidget(lb_SendRecvInfo);

    // 状态栏增加串口信息显示
    lb_ConnectInfo = new QLabel(tr("COM,110,8,1,None,None"));
    ui->statusbar->addWidget(lb_ConnectInfo);

    // 创建串口相关服务
    serialIOService = new SerialIOService();
    serialIOService->scanAvailableSerialPort(ui->cmb_PortName);
    serialIOService->addBaudItems(ui->cmb_BaudRate);
    ui->cmb_PortName->addItem("UDP");
    ui->cmb_PortName->addItem("TCPServer");
    ui->cmb_PortName->addItem("TCPClient");

    // 获得串口信息
    lb_ConnectInfo->setText(serialIOService->getConnectInfo());

    connect(serialIOService,&SerialIOService::readBytes,this,&MainWindow::onReadBytes);
    connect(serialIOService,&SerialIOService::sendBytesCount,this,&MainWindow::onSendCountChanged);
    connect(serialIOService,&SerialIOService::recvBytesCount,this,&MainWindow::onRecvCountChanged);


    // 多字节发送界面初始化
    multiSendInit();

    udpIOService = new UdpIOService();
    udpIOService->scanLocalAvlidAddr(ui->cmb_UdpLocalAddr);
    connect(udpIOService,&UdpIOService::readBytes,this,&MainWindow::onReadBytes);

    tcpServerIOService = new TcpServerIOService;
    tcpServerIOService->scanLocalAvlidAddr(ui->cmb_tcpServerLocalAddr);
    connect(tcpServerIOService,&TcpServerIOService::readBytes,this,&MainWindow::onReadBytes);
    connect(tcpServerIOService,&TcpServerIOService::addOneConnect,this,[this](QString ip,QString port){
        ui->cmb_tcpServerAimAddr->addItem(ip+':'+port);
        tcpServerConnectNum++;
        ui->cmb_tcpServerAimAddr->setItemText(0,QString("All Connect(%1)").arg(tcpServerConnectNum));
    });
    connect(tcpServerIOService,&TcpServerIOService::delOneConnect,this,[this](QString ip,QString port){
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


    tcpClientIOService = new TcpClientIOService();
    tcpClientIOService->scanLocalAvlidAddr(ui->cmb_tcpClientLocalAddr);
    connect(tcpClientIOService,&TcpClientIOService::readBytes,this,&MainWindow::onReadBytes);
    connect(tcpClientIOService,&TcpClientIOService::getLocalPort,this,[this](quint16 port){
        ui->le_tcpClientLocalPort->setText(QString::number(port));
    });

    loadSettings(settings);

    qApp->installNativeEventFilter(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadSettings(QSettings *settings)
{
    settings->beginGroup("MainWindow");
    auto serialPortName = settings->value("SerialPort","");
    auto serialBaud     = settings->value("SerialBaud","115200");
    auto cb_AddTimeStamp_state = settings->value("cb_AddTimeStamp","false");
    auto cb_RecvHexShow_state = settings->value("cb_RecvHexShow","false");
    auto cb_SendNewLine_state = settings->value("cb_SendNewLine","false");
    auto cb_SendByHex_state = settings->value("cb_SendByHex","false");
    auto cb_SendFeedback_state = settings->value("cb_SendFeedback","false");
    auto sendAreaStr = settings->value("sendAreaStr","");
    auto tcpClientLocalAddr = settings->value("tcpClientLocalAddr","");
    auto tcpServerLocalAddr = settings->value("tcpServerLocalAddr","");
    auto udpLocalAddr = settings->value("udpLocalAddr","");

    ui->cmb_PortName->setCurrentText(serialPortName.toString());
    serialIOService->setPortName(serialPortName.toString());
    ui->cmb_BaudRate->setCurrentText(serialBaud.toString());
    serialIOService->setBaudRate(serialBaud.toString());
    ui->cb_AddTimeStamp->setChecked(cb_AddTimeStamp_state.toBool());
    ui->cb_RecvHexShow->setChecked(cb_RecvHexShow_state.toBool());
    ui->cb_SendNewLine->setChecked(cb_SendNewLine_state.toBool());
    ui->cb_SendByHex->setChecked(cb_SendByHex_state.toBool());
    ui->cb_SendFeedback->setChecked(cb_SendFeedback_state.toBool());
    ui->plainTextEdit_send->insertPlainText(sendAreaStr.toString());
    ui->cmb_tcpClientLocalAddr->setCurrentText(tcpClientLocalAddr.toString());
    ui->cmb_tcpServerLocalAddr->setCurrentText(tcpServerLocalAddr.toString());
    ui->cmb_UdpLocalAddr->setCurrentText(udpLocalAddr.toString());

    settings->endGroup();
}

void MainWindow::saveSettings(QSettings *settings)
{
    settings->beginGroup("MainWindow");
    settings->setValue("SerialPort", ui->cmb_PortName->currentText());
    settings->setValue("SerialBaud", ui->cmb_BaudRate->currentText());
    settings->setValue("cb_AddTimeStamp",ui->cb_AddTimeStamp->isChecked());
    settings->setValue("cb_RecvHexShow",ui->cb_RecvHexShow->isChecked());
    settings->setValue("cb_SendNewLine",ui->cb_SendNewLine->isChecked());
    settings->setValue("cb_SendByHex",ui->cb_SendByHex->isChecked());
    settings->setValue("cb_SendFeedback",ui->cb_SendFeedback->isChecked());
    settings->setValue("sendAreaStr",ui->plainTextEdit_send->toPlainText());
    settings->setValue("tcpClientLocalAddr",ui->cmb_tcpClientLocalAddr->currentText());
    settings->setValue("tcpServerLocalAddr",ui->cmb_tcpServerLocalAddr->currentText());
    settings->setValue("udpLocalAddr",ui->cmb_UdpLocalAddr->currentText());

    settings->endGroup();
}

// 暂时未使用,功能不全，主要是想把不能显示的字符用空框显示，结果把换行都弄没了，后续处理
QString MainWindow::visualHex(const QByteArray &ba)
{
    QString out;
    out.reserve(ba.size() * 4);          // 预分配
    for (QChar c : ba) {
//        if (c < 0x20 || c == 0x7F)       // 控制字符
//            out += QChar(0x25A1);
//        else
            out += c;
    }
    return out;
}

// 读取串口数据后的处理
void MainWindow::onReadBytes(QByteArray bytes)
{
    QString strTemp="";
    if (ui->cb_AddTimeStamp->isChecked()) {
        strTemp = '\n' + QDateTime::currentDateTime()
                       .toString("[yy-MM-dd hh:mm:ss.zzz]收<-");
    }
    strTemp += ui->cb_RecvHexShow->isChecked() ? bytes.toHex(' ') : visualHex(bytes);
    ui->textBrowser_recv->insertPlainText(strTemp);
    ui->textBrowser_recv->moveCursor(QTextCursor::End);
    if(receiveFile.isOpen()){
        //receiveTextStream<<bytes;
        receiveFile.write(bytes);
    }
}

void MainWindow::onSendCountChanged(uint32_t count)
{
    sendCount += count;
    lb_SendRecvInfo->setText(QString("发送：%1 接收：%2").arg(sendCount, 8).arg(recvCount, 8));
}

void MainWindow::onRecvCountChanged(uint32_t count)
{
    recvCount += count;
    lb_SendRecvInfo->setText(QString("发送：%1 接收：%2").arg(sendCount, 8).arg(recvCount, 8));
}

// 串口关闭事件
void MainWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    saveSettings(settings);
}

// 打开串口
void MainWindow::on_btn_OpenSerial_clicked()
{
    if(serialIOService->isSerialOpen()){
        serialIOService->closeSerial();
        ui->cmb_PortName->setEnabled(true);
        ui->cmb_BaudRate->setEnabled(true);
        ui->btn_OpenSerial->setText("打开串口");
    }else{
        serialIOService->setPortName(ui->cmb_PortName->currentText());
        serialIOService->setBaudRate(ui->cmb_BaudRate->currentText());
        if(!serialIOService->openSerial()) return;
        ui->cmb_PortName->setEnabled(false);
        ui->cmb_BaudRate->setEnabled(false);
        ui->btn_OpenSerial->setText("关闭串口");
    }
}

void MainWindow::on_cmb_PortName_currentTextChanged(const QString &arg1)
{
    if(arg1 == "TCPClient"){
        ui->stackedWidget_Port->setCurrentIndex(3);
        lb_ConnectInfo->setText("TCP Client Mode");
        connectMode = MainWindow::TcpClient;
    }else if(arg1 == "TCPServer"){
        ui->stackedWidget_Port->setCurrentIndex(2);
        lb_ConnectInfo->setText("TCP Server Mode");
        connectMode = MainWindow::TcpServer;
    }else if(arg1 == "UDP"){
        ui->stackedWidget_Port->setCurrentIndex(1);
        lb_ConnectInfo->setText("UDP Mode");
        connectMode = MainWindow::Udp;
    }else{
        ui->stackedWidget_Port->setCurrentIndex(0);
        serialIOService->setPortName(arg1);
        lb_ConnectInfo->setText(serialIOService->getConnectInfo());
        connectMode = MainWindow::Serial;
    }
}


void MainWindow::on_cmb_BaudRate_currentTextChanged(const QString &arg1)
{
    if(connectMode == MainWindow::Serial)
    {
        serialIOService->setBaudRate(arg1);
        lb_ConnectInfo->setText(serialIOService->getConnectInfo());
    }
}

// 把发送的数据放到recv的文本框中，做回显
void MainWindow::onAppendSendBytes(QByteArray Bytes)
{
    if(ui->cb_SendFeedback->isChecked() == true){
        QString chunk;
        if (ui->cb_AddTimeStamp->isChecked()) {
            chunk = '\n' + QDateTime::currentDateTime()
                           .toString("[yy-MM-dd hh:mm:ss.zzz]发->");
        }
        chunk += ui->cb_RecvHexShow->isChecked() ? Bytes.toHex(' ') : visualHex(Bytes);
        ui->textBrowser_recv->insertPlainText(chunk);
        ui->textBrowser_recv->moveCursor(QTextCursor::End);
    }
}

// 多字符发送模式
void MainWindow::multiSendInit()
{
    for (int i = 0; i < 20; ++i) {
        Row row;

        row.check[0] = new QCheckBox();
        row.edit[0]  = new QLineEdit;
        row.btn[0]   = new QPushButton(QString("%1").arg(i + 1));
        row.check[1] = new QCheckBox();
        row.edit[1]  = new QLineEdit;
        row.btn[1]   = new QPushButton(QString("%1").arg(i + 1 + 20));

        // 把行号存到按钮里，方便后面知道是哪一行
        row.btn[0]->setProperty("row", i);
        row.btn[1]->setProperty("row", i + 20);

        ui->gridLayout->addWidget(row.check[0], i, 0);
        ui->gridLayout->addWidget(row.edit[0],  i, 1);
        ui->gridLayout->addWidget(row.btn[0],   i, 2);
        ui->gridLayout->addWidget(row.check[1], i, 3);
        ui->gridLayout->addWidget(row.edit[1],  i, 4);
        ui->gridLayout->addWidget(row.btn[1],   i, 5);

        rows.append(row);

        connect(row.btn[0], &QPushButton::clicked, this, [this, row]() {
            sendText(row.check[0],row.edit[0]->text());
        });
        connect(row.btn[1], &QPushButton::clicked, this, [this, row]() {
            sendText(row.check[1],row.edit[1]->text());
        });
    }
}

void MainWindow::sendText(QCheckBox *checkHexBox, QString text)
{
    if (checkHexBox->isChecked()) {
        // 1. 按十六进制发送
        QByteArray bytes = QByteArray::fromHex(
                               text.toUtf8().simplified());   // 去掉空格、回车
        if(ui->cb_SendNewLine->isChecked()){
            bytes.append(0x0D);
            bytes.append(0x0A);
        }
        sendBytes(bytes);
        onAppendSendBytes(bytes);
    } else {
        // 2. 按普通文本发送
        text.replace("\n", "\r\n");
        if(ui->cb_SendNewLine->isChecked()){
            text+="\r\n";
        }
        sendBytes(text.toLocal8Bit());
        onAppendSendBytes(text.toLocal8Bit());
    }
}

void MainWindow::sendBytes(QByteArray Bytes)
{
    if(connectMode == MainWindow::TcpClient){
        tcpClientIOService->sendBytes(Bytes);
    }else if(connectMode == MainWindow::TcpServer){
        tcpServerIOService->sendBytes(ui->cmb_tcpServerAimAddr->currentText(),Bytes);
    }else if(connectMode == MainWindow::Udp){
        udpIOService->sendBytes(Bytes);
    }else if(connectMode == MainWindow::Serial) {
        serialIOService->sendBytes(Bytes);
    }
}

// 点击发送按钮
void MainWindow::on_btn_Send_clicked()
{
    sendText(ui->cb_SendByHex,ui->plainTextEdit_send->toPlainText());
}


void MainWindow::on_btn_SelectFile_clicked()
{
    ui->le_SendFile->setText(QFileDialog::getOpenFileName(
                                 this,                       // 父窗口
                                 tr("选择文件"),             // 标题
                                 QDir::homePath(),           // 起始目录
                                 tr("所有文件 (*);;文本文件 (*.txt)")  // 过滤器
                             ));
}


void MainWindow::on_btn_SendFile_clicked()
{
    serialIOService->sendFile(ui->le_SendFile->text());
}

// 热插拔检测端口的代码
bool MainWindow::nativeEventFilter(const QByteArray & eventType, void * message, long * result)
{
    MSG* msg = reinterpret_cast<MSG*>(message);
    int msgType = msg->message;
    if (msgType == WM_DEVICECHANGE) {
        PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)msg->lParam;
        switch (msg->wParam) {
        case DBT_DEVICEARRIVAL:
            if (lpdb->dbch_devicetype == DBT_DEVTYP_PORT) {
//                PDEV_BROADCAST_PORT lpdbv = (PDEV_BROADCAST_PORT)lpdb;
//                QString port = QString::fromWCharArray(lpdbv->dbcp_name);//插入的串口名
                serialIOService->scanAvailableSerialPort(ui->cmb_PortName);
                ui->cmb_PortName->addItem("UDP");
                ui->cmb_PortName->addItem("TCPServer");
                ui->cmb_PortName->addItem("TCPClient");
            }
            break;
        case DBT_DEVICEREMOVECOMPLETE:
            if (lpdb->dbch_devicetype == DBT_DEVTYP_PORT) {
//                PDEV_BROADCAST_PORT lpdbv = (PDEV_BROADCAST_PORT)lpdb;
//                QString port = QString::fromWCharArray(lpdbv->dbcp_name);//拔出的串口名
                serialIOService->scanAvailableSerialPort(ui->cmb_PortName);
                ui->cmb_PortName->addItem("UDP");
                ui->cmb_PortName->addItem("TCPServer");
                ui->cmb_PortName->addItem("TCPClient");
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

// 保存文件的可选框按下
void MainWindow::on_cb_RecvToFile_clicked(bool checked)
{
    if(checked == true){
        QString filePath = QDir::currentPath() + "/receiveData_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".txt";

        receiveFile.setFileName(filePath);
        if(receiveFile.open(QIODevice::WriteOnly | QIODevice::Text)){
            receiveTextStream.setDevice(&receiveFile);
        }

        QMessageBox msg;
        msg.setText("文件将保存在"+filePath);
        msg.show();
        msg.exec();
    }else{
        if(receiveFile.isOpen()){
            receiveFile.close();
        }
    }
}


void MainWindow::on_radioButton_BasicSend_clicked()
{
    ui->stackedWidget_send->setCurrentIndex(0);
}


void MainWindow::on_radioButton_MultiSend_clicked()
{
    ui->stackedWidget_send->setCurrentIndex(1);
}


void MainWindow::on_rB_BasicComunication_clicked()
{
    ui->stackedWidget_Recv->setCurrentIndex(0);
}


void MainWindow::on_rB_WaveShow_clicked()
{
    ui->stackedWidget_Recv->setCurrentIndex(1);
}


void MainWindow::on_cmb_StopBits_currentTextChanged(const QString &arg1)
{
    serialIOService->setStopBits(arg1);
    if(connectMode == MainWindow::Serial) lb_ConnectInfo->setText(serialIOService->getConnectInfo());
}


void MainWindow::on_cmb_DataBits_currentTextChanged(const QString &arg1)
{
    serialIOService->setDataBits(arg1);
    if(connectMode == MainWindow::Serial) lb_ConnectInfo->setText(serialIOService->getConnectInfo());

}

void MainWindow::on_cmb_Parity_currentTextChanged(const QString &arg1)
{
    serialIOService->setParity(arg1);
    if(connectMode == MainWindow::Serial) lb_ConnectInfo->setText(serialIOService->getConnectInfo());
}


void MainWindow::on_btn_SendClear_clicked()
{
    ui->plainTextEdit_send->clear();
    sendCount = 0;
    lb_SendRecvInfo->setText(QString("发送：%1 接收：%2").arg(sendCount, 8).arg(recvCount, 8));
}


void MainWindow::on_btn_RecvClear_clicked()
{
    ui->textBrowser_recv->clear();
    recvCount = 0;
    lb_SendRecvInfo->setText(QString("发送：%1 接收：%2").arg(sendCount, 8).arg(recvCount, 8));
}


void MainWindow::on_action_version_triggered()
{
    QMessageBox msg;
    msg.setText("测试");
    msg.exec();
}


void MainWindow::on_btn_UdpConnect_clicked()
{
    if(ui->btn_UdpConnect->text() == "连接"){
        udpIOService->bindAimAddressAndPort(ui->le_UdpAimAddr->text(),ui->le_UdpAimPort->text(),
                                            ui->cmb_UdpLocalAddr->currentText(),ui->le_UdpLocalPort->text());
        ui->cmb_PortName->setEnabled(false);
        ui->le_UdpAimAddr->setEnabled(false);
        ui->le_UdpAimPort->setEnabled(false);
        ui->cmb_UdpLocalAddr->setEnabled(false);
        ui->le_UdpLocalPort->setEnabled(false);
        ui->btn_UdpConnect->setText("断开");
    }else if(ui->btn_UdpConnect->text() == "断开"){
        udpIOService->udpClose();
        ui->cmb_PortName->setEnabled(true);
        ui->le_UdpAimAddr->setEnabled(true);
        ui->le_UdpAimPort->setEnabled(true);
        ui->cmb_UdpLocalAddr->setEnabled(true);
        ui->le_UdpLocalPort->setEnabled(true);
        ui->btn_UdpConnect->setText("连接");
    }
}


void MainWindow::on_btn_Listen_clicked()
{
    if(ui->btn_Listen->text()=="侦听"){
        tcpServerIOService->setLocalAddrAndPort(ui->cmb_tcpServerLocalAddr->currentText(),ui->le_tcpServerLocalPort->text().toUInt());
        ui->btn_Listen->setText("断开");
        ui->cmb_PortName->setEnabled(false);
        ui->cmb_tcpServerLocalAddr->setEnabled(false);
        ui->le_tcpServerLocalPort->setEnabled(false);

    }else{
        tcpServerIOService->stopServer();
        ui->btn_Listen->setText("侦听");
        ui->cmb_PortName->setEnabled(true);
        ui->cmb_tcpServerLocalAddr->setEnabled(true);
        ui->le_tcpServerLocalPort->setEnabled(true);
    }

}


void MainWindow::on_btn_TcpClientConnect_clicked()
{
    if(ui->btn_TcpClientConnect->text()=="连接"){
        if(!tcpClientIOService->connectServer(ui->le_tcpClientAimAddr->text(),ui->le_tcpClientAimPort->text().toUInt()))
        {
            QMessageBox msg;
            msg.setText("Tcp Client Connect failed");
            msg.exec();
            return;
        }
        ui->btn_TcpClientConnect->setText("断开");
        ui->cmb_PortName->setEnabled(false);
        ui->le_tcpClientAimAddr->setEnabled(false);
        ui->le_tcpClientAimPort->setEnabled(false);
        ui->cmb_tcpClientLocalAddr->setEnabled(false);
    }else{
        tcpClientIOService->disconnectServer();
        ui->btn_TcpClientConnect->setText("连接");
        ui->cmb_PortName->setEnabled(true);
        ui->le_tcpClientAimAddr->setEnabled(true);
        ui->le_tcpClientAimPort->setEnabled(true);
        ui->cmb_tcpClientLocalAddr->setEnabled(true);
    }
}

