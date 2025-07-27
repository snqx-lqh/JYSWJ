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

    // 状态栏增加串口信息显示
    lb_ConnectInfo = new QLabel(tr("COM,110,8,1,None,None"));
    ui->statusbar->addWidget(lb_ConnectInfo);

    // 发送接收字符计数
    lb_SendRecvInfo= new QLabel(tr(""));
    lb_SendRecvInfo->setText(QString("发送：%1 接收：%2").arg(sendCount, 8).arg(recvCount, 8));
    ui->statusbar->addWidget(lb_SendRecvInfo);

    // 创建串口相关服务
    serialPort = new QSerialPort;
    serialIOService = new SerialIOService(serialPort);
    serialIOService->scanAvlidSerialPort(ui->cmb_PortName);
    serialIOService->addBaudItems(ui->cmb_BaudRate);

    // 获得串口信息
    lb_ConnectInfo->setText(serialIOService->getConnectInfo());

    connect(serialIOService,&SerialIOService::readBytes,this,&MainWindow::onReadBytes);
    connect(serialIOService,&SerialIOService::sendBytesCount,this,&MainWindow::onSendCountChanged);
    connect(serialIOService,&SerialIOService::recvBytesCount,this,&MainWindow::onRecvCountChanged);


    // 多字节发送界面初始化
    multiSendInit();

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

    ui->cmb_PortName->setCurrentText(serialPortName.toString());
    ui->cmb_BaudRate->setCurrentText(serialBaud.toString());
    ui->cb_AddTimeStamp->setChecked(cb_AddTimeStamp_state.toBool());
    ui->cb_RecvHexShow->setChecked(cb_RecvHexShow_state.toBool());
    ui->cb_SendNewLine->setChecked(cb_SendNewLine_state.toBool());
    ui->cb_SendByHex->setChecked(cb_SendByHex_state.toBool());
    ui->cb_SendFeedback->setChecked(cb_SendFeedback_state.toBool());
    ui->plainTextEdit_send->insertPlainText(sendAreaStr.toString());

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
    if(serialPort->isOpen()){
        serialPort->close();
        ui->cmb_PortName->setEnabled(true);
        ui->cmb_BaudRate->setEnabled(true);
        ui->btn_OpenSerial->setText("打开串口");
    }else{
        serialPort->setPortName(ui->cmb_PortName->currentText());
        serialPort->setBaudRate(ui->cmb_BaudRate->currentText().toUInt());

        int connect = serialPort->open(QSerialPort::ReadWrite);
        if(connect){
            qDebug() << "串口打开成功";
        }else{
            qDebug() << "串口打开失败";
            QMessageBox msg;
            msg.setText("串口打开失败");
            msg.show();
            msg.exec();
            return;
        }
        ui->cmb_PortName->setEnabled(false);
        ui->cmb_BaudRate->setEnabled(false);
        ui->btn_OpenSerial->setText("关闭串口");
    }
}

void MainWindow::on_cmb_PortName_currentTextChanged(const QString &arg1)
{
    serialPort->setPortName(arg1);
    lb_ConnectInfo->setText(serialIOService->getConnectInfo());
}


void MainWindow::on_cmb_BaudRate_currentTextChanged(const QString &arg1)
{
    serialPort->setBaudRate(arg1.toUInt());
    lb_ConnectInfo->setText(serialIOService->getConnectInfo());
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

        connect(row.btn[0], &QPushButton::clicked, this, [this, row, i]() {
            if (row.check[0]->isChecked()) {
                // 1. 按十六进制发送
                QByteArray bytes = QByteArray::fromHex(
                                       rows[i].edit[0]->text()
                                       .toUtf8().simplified());   // 去掉空格、回车
                serialIOService->sendBytes(bytes);
                onAppendSendBytes(bytes);
            } else {
                // 2. 按普通文本发送
                QString text = rows[i].edit[0]->text();
                text.replace("\n", "\r\n");
                serialIOService->sendBytes(text.toLocal8Bit());
                onAppendSendBytes(text.toLocal8Bit());
            }
        });
        connect(row.btn[1], &QPushButton::clicked, this, [this, row, i]() {
            if (row.check[1]->isChecked()) {
                // 1. 按十六进制发送
                QByteArray bytes = QByteArray::fromHex(
                                       rows[i].edit[1]->text()
                                       .toUtf8().simplified());   // 去掉空格、回车
                serialIOService->sendBytes(bytes);
                onAppendSendBytes(bytes);
            } else {
                // 2. 按普通文本发送
                QString text = rows[i].edit[1]->text();
                text.replace("\n", "\r\n");
                serialIOService->sendBytes(text.toLocal8Bit());
                onAppendSendBytes(text.toLocal8Bit());
            }
        });
    }
}

// 点击发送按钮
void MainWindow::on_btn_Send_clicked()
{
    if (ui->cb_SendByHex->isChecked()) {
        // 1. 按十六进制发送
        QByteArray bytes = QByteArray::fromHex(
                               ui->plainTextEdit_send->toPlainText()
                               .toUtf8().simplified());   // 去掉空格、回车
        serialIOService->sendBytes(bytes);
        onAppendSendBytes(bytes);
    } else {
        // 2. 按普通文本发送
        QString text = ui->plainTextEdit_send->toPlainText();
        text.replace("\n", "\r\n");
        serialIOService->sendBytes(text.toLocal8Bit());
        onAppendSendBytes(text.toLocal8Bit());
    }
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
                serialIOService->scanAvlidSerialPort(ui->cmb_PortName);
            }
            break;
        case DBT_DEVICEREMOVECOMPLETE:
            if (lpdb->dbch_devicetype == DBT_DEVTYP_PORT) {
//                PDEV_BROADCAST_PORT lpdbv = (PDEV_BROADCAST_PORT)lpdb;
//                QString port = QString::fromWCharArray(lpdbv->dbcp_name);//拔出的串口名
                serialIOService->scanAvlidSerialPort(ui->cmb_PortName);
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
    uint8_t openFlag = 0;
    if (serialPort->isOpen()){
        openFlag = 1;
        serialPort->close();
    }
    if (arg1 == "1")  serialPort->setStopBits(QSerialPort::OneStop);
    else if(arg1 == "1.5")  serialPort->setStopBits(QSerialPort::OneAndHalfStop);
    else if(arg1 == "2")  serialPort->setStopBits(QSerialPort::TwoStop);
    lb_ConnectInfo->setText(serialIOService->getConnectInfo());
    if(openFlag == 1) serialPort->open(QSerialPort::ReadWrite);
}


void MainWindow::on_cmb_DataBits_currentTextChanged(const QString &arg1)
{
    uint8_t openFlag = 0;
    if (serialPort->isOpen()){
        openFlag = 1;
        serialPort->close();
    }
    if (arg1 == "8")  serialPort->setDataBits(QSerialPort::Data8);
    else if(arg1 == "7")  serialPort->setBaudRate(QSerialPort::Data7);
    else if(arg1 == "6")  serialPort->setBaudRate(QSerialPort::Data6);
    else if(arg1 == "5")  serialPort->setBaudRate(QSerialPort::Data5);
    lb_ConnectInfo->setText(serialIOService->getConnectInfo());
    if(openFlag == 1) serialPort->open(QSerialPort::ReadWrite);
}

void MainWindow::on_cmb_Parity_currentTextChanged(const QString &arg1)
{
    uint8_t openFlag = 0;
    if (serialPort->isOpen()){
        openFlag = 1;
        serialPort->close();
    }
    if (arg1 == "None")  serialPort->setParity(QSerialPort::NoParity);
    else if(arg1 == "Odd")  serialPort->setParity(QSerialPort::OddParity);
    else if(arg1 == "Even")  serialPort->setParity(QSerialPort::EvenParity);
    lb_ConnectInfo->setText(serialIOService->getConnectInfo());
    if(openFlag == 1) serialPort->open(QSerialPort::ReadWrite);
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

