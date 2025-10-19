#include "protocoltransferform.h"
#include "ui_protocoltransferform.h"

ProtocolTransferForm::ProtocolTransferForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProtocolTransferForm)
{
    ui->setupUi(this);

    QDir dir(QCoreApplication::applicationDirPath());
    m_iniFile = dir.filePath("Config/settings.ini");

    connect(&MainTimer,&QTimer::timeout,this,&ProtocolTransferForm::onMainTimeout);
    MainTimer.setInterval(10);

    send_state = ProtocolTransferForm::IDLE;
    ui->comboBox_HistoryFile->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    loadSettings();
}

ProtocolTransferForm::~ProtocolTransferForm()
{
    saveSettings();
    delete ui;
}

void ProtocolTransferForm::showMsg(QString color, QString msg)
{
    QString timeStr = QDateTime::currentDateTime().toString("[yy-MM-dd hh:mm:ss]");
    msg = timeStr + msg;

    QString html = QStringLiteral("<font color=\"%1\">%2</font>")
                   .arg(color.toHtmlEscaped(), msg.toHtmlEscaped());
    ui->plainTextEdit->appendHtml(html);
}

quint16 ProtocolTransferForm::crc16_ccitt(const quint8 *ptr, qint32 len)
{
    unsigned int i;
    unsigned short crc = 0x0000;

    while(len--)
    {
        crc ^= (unsigned short)(*ptr++) << 8;
        for (i = 0; i < 8; ++i)
        {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }

    return crc;
}


void ProtocolTransferForm::XmodemTransfer()
{
    uint8_t buf[1034];  // 临时存储变量

    if(ui->cmb_SendMode->currentText() == "Xmodem 128"){
        kPayload   = 128;
        buf[0] = 0x01;               // SOH
    }else if(ui->cmb_SendMode->currentText() == "Xmodem 1024"){
        kPayload   = 1024;
        buf[0] = 0x02;               // STX
    }

    // 这是全局变量 从1到结束
    packetNum++;
    // 计算当前发送内容的偏移值
    int offset = (packetNum - 1) * kPayload;
    // 包序 和 包序反码
    buf[1] = packetNum & 0xFF;
    buf[2] = 0xFF - buf[1];
    // 拷贝 1024 字节数据，不足时用 0x1A 填充
    int bytesToCopy = qMin(kPayload, XmodeArray.size() - offset);
    memcpy(buf + 3, XmodeArray.constData() + offset, bytesToCopy);
    if (bytesToCopy < kPayload)
        memset(buf + 3 + bytesToCopy, 0x1A, kPayload - bytesToCopy);

    // CRC16 校验（放在包尾）
    quint16 crc = crc16_ccitt(buf + 3, kPayload);
    if(ui->cmb_SendMode->currentText() == "Xmodem 128"){
        buf[131] = (crc >> 8) & 0xFF;
        buf[132] =  crc       & 0xFF;
    }else if(ui->cmb_SendMode->currentText() == "Xmodem 1024"){
        buf[1027] = (crc >> 8) & 0xFF;
        buf[1028] =  crc       & 0xFF;
    }

    // 发送本包
    emit sendBytes(QByteArray(reinterpret_cast<const char*>(buf), kPayload+5));

    // 将本次发送的数量进行记录，放到进度条
    XmodemSendCount += bytesToCopy;
    ui->progressBar->setValue(XmodemSendCount * 100/XmodeArray.size());
    qDebug()<<"send:"<<packetNum * kPayload<<" all"<< XmodeArray.size();
    // 状态进入发送完成状态
    if((int32_t)(packetNum * kPayload) > XmodeArray.size())
    {
        send_state = ProtocolTransferForm::XMODEM_SEND_ALL_FINISH;
    }else{
        send_state = ProtocolTransferForm::XMODEM_SEND_DOWN;
    }
}

void ProtocolTransferForm::loadSettings()
{
    QSettings settings(m_iniFile,QSettings::IniFormat);
    settings.beginGroup("ProtocolTransferForm");

    int History_Count = settings.value("History_Count",0).toInt();

    for(int i=0;i<History_Count;i++){
        QString temp = settings.value(QString("History_File%1").arg(i)).toString();
        bool exists = QFileInfo::exists(temp);
        if(exists)
            ui->comboBox_HistoryFile->addItem(temp);
    }
    ui->cmb_SendMode->setCurrentText(settings.value("cmb_SendMode",ui->cmb_SendMode->currentText()).toString());
    ui->comboBox_HistoryFile->setCurrentText(settings.value("History_File",ui->comboBox_HistoryFile->currentText()).toString());
    settings.endGroup();
}

void ProtocolTransferForm::saveSettings()
{
    QSettings settings(m_iniFile,QSettings::IniFormat);
    settings.beginGroup("ProtocolTransferForm");

    settings.setValue("History_Count",ui->comboBox_HistoryFile->count());
    settings.setValue("History_File",ui->comboBox_HistoryFile->currentText());
    for(int i=0;i<ui->comboBox_HistoryFile->count();i++){
        settings.setValue(QString("History_File%1").arg(i),ui->comboBox_HistoryFile->itemText(i));
    }
    settings.setValue("cmb_SendMode",ui->cmb_SendMode->currentText());
    settings.endGroup();
}

void ProtocolTransferForm::onMainTimeout()
{
    switch (send_state) {
    case ProtocolTransferForm::IDLE:
        break;
    case ProtocolTransferForm::XMODEM_SEND:
        XmodemTransfer();
        break;
    case ProtocolTransferForm::XMODEM_SEND_SEND_EOT:
        {
            QByteArray bytes;
            bytes.append(0x04);
            emit sendBytes(bytes);
            send_state = XMODEM_SEND_WAIT_EOT_ACK;
            break;
        }
    default:
        break;
    }
}

void ProtocolTransferForm::onReadBytes(QByteArray bytes)
{
    if(startTransfer)
    {
        qDebug()<<"Recv:"<<bytes;
        if(bytes[0] == 'C' && send_state == ProtocolTransferForm::IDLE){
            send_state = XMODEM_SEND;
            showMsg("green",QString("Start Send..."));
        }else if((uint8_t)bytes[0] == 0x06 && send_state == ProtocolTransferForm::XMODEM_SEND_DOWN){
            send_state = XMODEM_SEND;
        }else if((uint8_t)bytes[0] == 0x06 && send_state == ProtocolTransferForm::XMODEM_SEND_ALL_FINISH){
            send_state = XMODEM_SEND_SEND_EOT;
        }else if((uint8_t)bytes[0] == 0x06 && send_state == ProtocolTransferForm::XMODEM_SEND_WAIT_EOT_ACK){
            send_state = IDLE;
            showMsg("green",QString("发送完成！"));
            startTransfer = false;
            MainTimer.stop();
            ui->progressBar->setValue(0);
        }
    }
}

void ProtocolTransferForm::on_btn_CleanWindow_clicked()
{
    ui->plainTextEdit->clear();
}


void ProtocolTransferForm::on_btn_selectFile_clicked()
{
    QString SelectFile = QFileDialog::getOpenFileName(
                this,                       // 父窗口
                tr("选择文件"),             // 标题
                lastDir,
                tr("所有文件 (*);;文本文件 (*.txt)")  // 过滤器
            );
    if(SelectFile.isEmpty()) return;
    ui->comboBox_HistoryFile->addItem(SelectFile);
    ui->comboBox_HistoryFile->setCurrentText(SelectFile);
    if(ui->comboBox_HistoryFile->count()>5){
        ui->comboBox_HistoryFile->removeItem(0);
    }
    lastDir = QFileInfo(SelectFile).absolutePath();
}


void ProtocolTransferForm::on_btn_StartSend_clicked()
{

    file.setFileName(ui->comboBox_HistoryFile->currentText());
    if(!file.open(QIODevice::ReadOnly)){
        showMsg("green",QString("打开文件失败"));
        file.close();
        return;
    }
    XmodeArray = file.readAll();
    file.close();
    showMsg("green",QString("开始传输文件：%1").arg(ui->comboBox_HistoryFile->currentText()));
    showMsg("green",QString("Wait C..."));
    startTransfer = true;
    send_state = ProtocolTransferForm::IDLE;
    MainTimer.start();
    XmodemSendCount = 0;
    packetNum = 0;
}


void ProtocolTransferForm::on_btn_CancelSend_clicked()
{
    if(startTransfer == true){
        startTransfer = false;
        MainTimer.stop();
        QByteArray bytes;
        bytes.append(0x18);
        emit sendBytes(bytes);
        XmodemSendCount = 0;
        packetNum = 0;
        ui->progressBar->setValue(0);
        send_state = ProtocolTransferForm::IDLE;
        showMsg("red",QString("取消传输"));
    }
}



