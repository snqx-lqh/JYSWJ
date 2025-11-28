#include "ymodem.h"

Ymodem::Ymodem(QObject *parent) : QObject(parent)
{
    connect(&MainTimer,&QTimer::timeout,this,&Ymodem::onMainTimeout);
    MainTimer.setInterval(10);
}

void Ymodem::StartSendYmodem(QString YmodemMode, QString FilePath)
{
    mYmodemMode = YmodemMode;
    file.setFileName(FilePath);
    if(!file.open(QIODevice::ReadOnly)){
        emit ymodemStateChange(SendError,QString("打开文件失败"));
        file.close();
        return;
    }
    YmodeArray = file.readAll();
    file.close();
    emit ymodemStateChange(SendInfo,QString("开始传输文件：%1").arg(FilePath));
    emit ymodemStateChange(SendInfo,QString("Wait C..."));
    startTransfer = true;
    send_state = Ymodem::WAIT_C;
    MainTimer.start();
    YmodemSendCount = 0;
    packetNum = 0;
}

void Ymodem::CancelSendYmodem()
{
    if(startTransfer == true){
        startTransfer = false;
        MainTimer.stop();
        QByteArray bytes;
        bytes.append(0x18);
        emit sendBytes(bytes);
        YmodemSendCount = 0;
        packetNum = 0;
        emit ymodemStateChange(SendPercent,QString::number(0));
        send_state = Ymodem::IDLE;
        emit ymodemStateChange(SendInfo,QString("取消传输"));
    }
}

quint16 Ymodem::crc16_ccitt(const quint8 *ptr, qint32 len)
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

void Ymodem::YmodemTransferHead()
{
    uint8_t buf[1034]={0};  // 临时存储变量

    if(mYmodemMode == "Ymodem 128"){
        kPayload   = 128;
        buf[0] = 0x01;               // SOH
    }else if(mYmodemMode == "Ymodem 1024"){
        kPayload   = 1024;
        buf[0] = 0x02;               // STX
    }

    // 包序 和 包序反码
    buf[1] = 0x00;
    buf[2] = 0xFF;
    // 拷贝 1024 字节数据，不足时用 0x1A 填充
    QFileInfo fileInfo(file);
    QByteArray fileNameHead = fileInfo.fileName().toUtf8();
    fileNameHead.append('\0');
    QString fileSize = QString::number(fileInfo.size());
    fileNameHead.append(fileSize.toLocal8Bit());
    memcpy(buf + 3, fileNameHead.constData(), fileNameHead.length());

    // CRC16 校验（放在包尾）
    quint16 crc = crc16_ccitt(buf + 3, kPayload);
    if(mYmodemMode == "Ymodem 128"){
        buf[131] = (crc >> 8) & 0xFF;
        buf[132] =  crc       & 0xFF;
    }else if(mYmodemMode == "Ymodem 1024"){
        buf[1027] = (crc >> 8) & 0xFF;
        buf[1028] =  crc       & 0xFF;
    }

    // 发送本包
    emit sendBytes(QByteArray(reinterpret_cast<const char*>(buf), kPayload+5));


}

void Ymodem::YmodemTransferEnd()
{
    uint8_t buf[1034]={0};  // 临时存储变量

    if(mYmodemMode == "Ymodem 128"){
        kPayload   = 128;
        buf[0] = 0x01;               // SOH
    }else if(mYmodemMode == "Ymodem 1024"){
        kPayload   = 1024;
        buf[0] = 0x02;               // STX
    }
    // 包序 和 包序反码
    buf[1] = 0x00;
    buf[2] = 0xFF;
    // CRC16 校验（放在包尾）
    quint16 crc = crc16_ccitt(buf + 3, kPayload);
    if(mYmodemMode == "Ymodem 128"){
        buf[131] = (crc >> 8) & 0xFF;
        buf[132] =  crc       & 0xFF;
    }else if(mYmodemMode == "Ymodem 1024"){
        buf[1027] = (crc >> 8) & 0xFF;
        buf[1028] =  crc       & 0xFF;
    }
    // 发送本包
    emit sendBytes(QByteArray(reinterpret_cast<const char*>(buf), kPayload+5));
}

void Ymodem::YmodemTransfer()
{
    uint8_t buf[1034];  // 临时存储变量

    if(mYmodemMode == "Ymodem 128"){
        kPayload   = 128;
        buf[0] = 0x01;               // SOH
    }else if(mYmodemMode == "Ymodem 1024"){
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
    int bytesToCopy = qMin(kPayload, YmodeArray.size() - offset);
    memcpy(buf + 3, YmodeArray.constData() + offset, bytesToCopy);
    if (bytesToCopy < kPayload)
        memset(buf + 3 + bytesToCopy, 0x1A, kPayload - bytesToCopy);

    // CRC16 校验（放在包尾）
    quint16 crc = crc16_ccitt(buf + 3, kPayload);
    if(mYmodemMode == "Ymodem 128"){
        buf[131] = (crc >> 8) & 0xFF;
        buf[132] =  crc       & 0xFF;
    }else if(mYmodemMode == "Ymodem 1024"){
        buf[1027] = (crc >> 8) & 0xFF;
        buf[1028] =  crc       & 0xFF;
    }

    // 发送本包
    emit sendBytes(QByteArray(reinterpret_cast<const char*>(buf), kPayload+5));

    // 将本次发送的数量进行记录，放到进度条
    YmodemSendCount += bytesToCopy;
    emit ymodemStateChange(SendPercent,QString::number(YmodemSendCount * 100/YmodeArray.size()));

    // 状态进入发送完成状态
    if((int32_t)(packetNum * kPayload) > YmodeArray.size())
    {
        send_state = Ymodem::YMODEM_SEND_ALL_FINISH;
    }else{
        send_state = Ymodem::YMODEM_SEND_DOWN;
    }
}

void Ymodem::onReadBytes(QByteArray bytes)
{
    qDebug()<<"startTransfer:"<<startTransfer<<"Ymodem::onReadBytes:"<<bytes;


    if(startTransfer)
    {
        m_ringBuffer.write((char*)bytes.constData(),bytes.length());
        if(bytes.contains(0x18)){
            send_state = IDLE;
            startTransfer = false;
            emit ymodemStateChange(SendPercent,QString::number(0));
            emit ymodemStateChange(SendTransferState,QString("1"));
            MainTimer.stop();
            m_ringBuffer.clear();
        }
    }
    //qDebug()<< "out  state:"<<send_state;
}

void Ymodem::onYmodemStateChange(YmodemState type, QString state)
{
    qDebug()<< "Ymodem::onYmodemStateChange:"<<type<<" :"<<state;
    if(type == IOConnectState){
        if(state == "0"){
            IOConnect = false;
        }else {
            IOConnect = true;
        }
    }
}

void Ymodem::onMainTimeout()
{
    if(startTransfer){
        if(IOConnect == false){
            emit ymodemStateChange(SendTransferState,QString("DisConnected"));
            send_state = IDLE;
            startTransfer = false;
            MainTimer.stop();
            return ;
        }
    }

    switch (send_state) {
        case Ymodem::IDLE:
            break;
        case Ymodem::WAIT_C:
            if(m_ringBuffer.dataSize()!=0){
                char data = m_ringBuffer.at(0);
                if(data == 'C'){
                    send_state = YMODEM_SEND_HEAD;
                }
                m_ringBuffer.advanceReadPos(1);
            }
            break;
        case Ymodem::YMODEM_SEND_HEAD:
            YmodemTransferHead();
            qDebug()<< "state:"<<send_state;
            send_state = Ymodem::YMODEM_SEND_HEAD_WAIT_CA;
            break;
        case Ymodem::YMODEM_SEND_HEAD_WAIT_CA:
            if(m_ringBuffer.dataSize()!=0){
                qDebug()<< "state:"<<send_state;
                if(m_ringBuffer.dataSize() >= 2){
                    char data1 = m_ringBuffer.at(0);
                    char data2 = m_ringBuffer.at(1);
                    if(data1 == 0X06 && data2 == 'C'){
                        send_state = YMODEM_SEND;
                        m_ringBuffer.advanceReadPos(2);
                    }else{
                        m_ringBuffer.advanceReadPos(1);
                    }
                }
            }
            break;
        case Ymodem::YMODEM_SEND:
            YmodemTransfer();
            qDebug()<< "state:"<<send_state;
            break;
        case Ymodem::YMODEM_SEND_DOWN:
            if(m_ringBuffer.dataSize()!=0){
                qDebug()<< "state:"<<send_state;
                char data = m_ringBuffer.at(0);
                if(data == 0X06){
                    send_state = YMODEM_SEND;
                }
                m_ringBuffer.advanceReadPos(1);
            }
            break;
        case Ymodem::YMODEM_SEND_ALL_FINISH:
            if(m_ringBuffer.dataSize()!=0){
                qDebug()<< "state:"<<send_state;
                char data = m_ringBuffer.at(0);
                if(data == 0X06){
                    send_state = YMODEM_SEND_SEND_EOT;
                }
                m_ringBuffer.advanceReadPos(1);
            }
        break;
        case Ymodem::YMODEM_SEND_SEND_EOT:
            {
                QByteArray bytes;
                bytes.append(0x04);
                emit sendBytes(bytes);
                send_state = YMODEM_SEND_WAIT_EOT_ACK;
                break;
            }
        case Ymodem::YMODEM_SEND_SEND_EOT_2:
            {
                QByteArray bytes;
                bytes.append(0x04);
                emit sendBytes(bytes);
                send_state = YMODEM_SEND_WAIT_EOT_ACK;
                break;
            }
        case Ymodem::YMODEM_SEND_WAIT_EOT_ACK:
            if(m_ringBuffer.dataSize()!=0){
                qDebug()<< "state:"<<send_state;
                char data = m_ringBuffer.at(0);
                if(data == 0X06){
                    send_state = YMODEM_SEND_WAIT_NEXT_FILE_C;
                }else if(data == 0X15){
                    send_state = YMODEM_SEND_SEND_EOT_2;
                }
                m_ringBuffer.advanceReadPos(1);
            }
            break;
        case Ymodem::YMODEM_SEND_WAIT_NEXT_FILE_C:
            // 本文件由于只处理一个文件，所以直接到结尾，但是正常来说，应该
            // 在接收到 C 后有个处理 判断是否还有文件，如果还有文件，需要再转到开头
            if(m_ringBuffer.dataSize()!=0){
                qDebug()<< "state:"<<send_state;
                char data = m_ringBuffer.at(0);
                if(data == 'C'){
                    send_state = YMODEM_SEND_END;
                }
                m_ringBuffer.advanceReadPos(1);
            }
            break;
        case Ymodem::YMODEM_SEND_END:
            YmodemTransferEnd();
            send_state = YMODEM_SEND_END_WAIT_ACK;
        break;
        case Ymodem::YMODEM_SEND_END_WAIT_ACK:
            if(m_ringBuffer.dataSize()!=0){
                qDebug()<< "state:"<<send_state;
                char data = m_ringBuffer.at(0);
                if(data == 0X06){
                    send_state = IDLE;
                    startTransfer = false;
                    emit ymodemStateChange(SendPercent,QString::number(0));
                    emit ymodemStateChange(SendTransferState,QString("1"));
                    MainTimer.stop();
                    m_ringBuffer.clear();
                }else{
                    m_ringBuffer.advanceReadPos(1);
                }
            }
        break;
        default:
            break;
        }
}
