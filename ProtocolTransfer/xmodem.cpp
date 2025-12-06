#include "xmodem.h"

Xmodem::Xmodem(QObject *parent) : QObject(parent)
{
    connect(&MainTimer,&QTimer::timeout,this,&Xmodem::onMainTimeout);
    MainTimer.setInterval(10);
}

void Xmodem::StartSendXmodem(QString XmodemMode, QString FilePath)
{
    mXmodemMode = XmodemMode;
    file.setFileName(FilePath);
    if(!file.open(QIODevice::ReadOnly)){
        emit xmodemStateChange(SendError,QString("打开文件失败"));
        file.close();
        return;
    }
    XmodeArray = file.readAll();
    file.close();
    emit xmodemStateChange(SendInfo,QString("开始传输文件：%1").arg(FilePath));
    emit xmodemStateChange(SendInfo,QString("Wait C..."));
    startTransfer = true;
    send_state = Xmodem::WAIT_C;
    MainTimer.start();
    XmodemSendCount = 0;
    packetNum = 0;
}

void Xmodem::CancelSendXmodem()
{
    if(startTransfer == true){
        startTransfer = false;
        MainTimer.stop();
        QByteArray bytes;
        bytes.append(0x18);
        emit sendBytes(bytes);
        XmodemSendCount = 0;
        packetNum = 0;
        emit xmodemStateChange(SendPercent,QString::number(0));
        send_state = Xmodem::IDLE;
        emit xmodemStateChange(SendInfo,QString("取消传输"));
    }
}

void Xmodem::onReadBytes(QByteArray bytes)
{
    if(startTransfer)
    {
        m_ringBuffer.write((char*)bytes.constData(),bytes.length());
        if(bytes.contains(0x18)){
            send_state = IDLE;
            startTransfer = false;
            emit xmodemStateChange(SendPercent,QString::number(0));
            emit xmodemStateChange(SendTransferState,QString("1"));
            MainTimer.stop();
            m_ringBuffer.clear();
        }
    }
}

void Xmodem::onXmodemStateChange(XmodemState type, QString state)
{
    qDebug()<< "Xmodem::onXmodemStateChange:"<<type<<" :"<<state;
    if(type == IOConnectState){
        if(state == "0"){
            IOConnect = false;
        }else {
            IOConnect = true;
        }
    }
}

void Xmodem::onMainTimeout()
{
    if(startTransfer){
        if(IOConnect == false){
            emit xmodemStateChange(SendTransferState,QString("DisConnected"));
            send_state = IDLE;
            startTransfer = false;
            MainTimer.stop();
            return ;
        }
    }

    switch (send_state) {
        case Xmodem::IDLE:
            break;
        case Xmodem::WAIT_C:
            if(m_ringBuffer.dataSize()!=0){
                qDebug()<< "state:"<<send_state;
                char data = m_ringBuffer.at(0);
                if(data == 'C'){
                    send_state = XMODEM_SEND;
                }
                m_ringBuffer.advanceReadPos(1);
            }
            retryCount = 0;
            break;
        case Xmodem::XMODEM_SEND:
            XmodemTransfer(false);
            send_state = Xmodem::XMODEM_SEND_DOWN;
            break;
        case Xmodem::XMODEM_SEND_RETRY:
            XmodemTransfer(true);
            retryCount++;
            emit xmodemStateChange(SendInfo,QString("重发第%1次").arg(retryCount));
            if(retryCount > 5){
                send_state = Xmodem::IDLE;
                startTransfer = false;
                emit xmodemStateChange(SendPercent,QString::number(0));
                emit xmodemStateChange(SendTransferState,QString("2"));
                MainTimer.stop();
                m_ringBuffer.clear();
            }
            send_state = Xmodem::XMODEM_SEND_DOWN;
            break;
        case Xmodem::XMODEM_SEND_DOWN:
            if(m_ringBuffer.dataSize()!=0){
                qDebug()<< "state:"<<send_state;
                char data = m_ringBuffer.at(0);
                if(data == XmodemCMD::ACK){
                    // 状态进入发送完成状态
                    if((int32_t)(packetNum * kPayload) > XmodeArray.size())
                    {
                        send_state = Xmodem::XMODEM_SEND_SEND_EOT;
                    }else{
                        send_state = Xmodem::XMODEM_SEND;
                    }
                    retryCount = 0;
                }else if(data == XmodemCMD::NAK){
                    send_state = Xmodem::XMODEM_SEND_RETRY;
                }
                m_ringBuffer.advanceReadPos(1);
            }
            break;
        case Xmodem::XMODEM_SEND_SEND_EOT:
            {
                QByteArray bytes;
                bytes.append(XmodemCMD::EOT);
                emit sendBytes(bytes);
                send_state = XMODEM_SEND_WAIT_EOT_ACK;
                break;
            }
        case Xmodem::XMODEM_SEND_WAIT_EOT_ACK:
            if(m_ringBuffer.dataSize()!=0){
                qDebug()<< "state:"<<send_state;
                char data = m_ringBuffer.at(0);
                if(data == XmodemCMD::ACK){
                    send_state = IDLE;
                    startTransfer = false;
                    emit xmodemStateChange(SendPercent,QString::number(0));
                    emit xmodemStateChange(SendTransferState,QString("1"));
                    MainTimer.stop();
                    m_ringBuffer.clear();
                }
                m_ringBuffer.advanceReadPos(1);
            }
        break;
        default:
            break;
        }
}

quint16 Xmodem::crc16_ccitt(const quint8 *ptr, qint32 len)
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


void Xmodem::XmodemTransfer(bool retry)
{
    uint8_t buf[1034];  // 临时存储变量

    if(mXmodemMode == "Xmodem 128"){
        kPayload   = 128;
        buf[0] = 0x01;               // SOH
    }else if(mXmodemMode == "Xmodem 1024"){
        kPayload   = 1024;
        buf[0] = 0x02;               // STX
    }

    // 这是全局变量 从1到结束
    if(retry == false)
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
    if(mXmodemMode == "Xmodem 128"){
        buf[131] = (crc >> 8) & 0xFF;
        buf[132] =  crc       & 0xFF;
    }else if(mXmodemMode == "Xmodem 1024"){
        buf[1027] = (crc >> 8) & 0xFF;
        buf[1028] =  crc       & 0xFF;
    }

    // 发送本包
    emit sendBytes(QByteArray(reinterpret_cast<const char*>(buf), kPayload+5));

    // 将本次发送的数量进行记录，放到进度条
    if(retry == false)
        XmodemSendCount += bytesToCopy;
    emit xmodemStateChange(SendPercent,QString::number(XmodemSendCount * 100/XmodeArray.size()));
}

