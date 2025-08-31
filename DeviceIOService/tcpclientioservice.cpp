#include "tcpclientioservice.h"

TcpClientIOService::TcpClientIOService(QObject *parent) : QObject      (parent)
{
    //给客户端指针实例化空间
    tcpClientSocket = new QTcpSocket;
    connect(tcpClientSocket,&QTcpSocket::readyRead,this,&TcpClientIOService::onReadReady);

    connect(tcpClientSocket, &QTcpSocket::connected, this, &TcpClientIOService::connected_slot);
    connect(tcpClientSocket, &QTcpSocket::disconnected, this, &TcpClientIOService::disconnected_slot);
}

bool TcpClientIOService::isTcpClientOpen()
{
    return tcpClientSocket->isOpen();
}

bool TcpClientIOService::connectServer(QString hostName, quint32 port)
{
    /* 1. 端口范围限制 */
    if (port == 0 || port > 65535) {
        qWarning() << "Port out of range:" << port;
        return false;
    }

    /* 2. 地址格式检查：支持 IPv4 / IPv6 / 域名 */
    QHostAddress addr;
    if (!addr.setAddress(hostName)) {                 // 不是纯 IP
        // 做一次简单的域名格式校验，只让“合法字符”通过
        QRegularExpression re(R"(^[\w.-]+$)");
        if (!re.match(hostName).hasMatch()) {
            qWarning() << "Invalid host name format:" << hostName;
            return false;
        }
    } else {
        /* 可选：强制只接受 IPv4（如需支持 IPv6 去掉即可） */
        if (addr.protocol() != QAbstractSocket::IPv4Protocol) {
            qWarning() << "Only IPv4 is allowed";
            return false;
        }
    }

    /* 3. 如果 socket 还没断开，先断开再连 */
    if (tcpClientSocket->state() != QAbstractSocket::UnconnectedState) {
        tcpClientSocket->abort();
    }

    /* 4. 真正发起连接（异步） */
    tcpClientSocket->connectToHost(hostName, port);
    return true;   // 后续通过 connected()/errorOccurred() 信号判断结果
}

void TcpClientIOService::disconnectServer()
{
    tcpClientSocket->disconnectFromHost();
}

void TcpClientIOService::sendBytes(QByteArray bytes)
{
    tcpClientSocket->write(bytes);
    emit sendBytesCount(bytes.length());
}

void TcpClientIOService::sendFile(QString fileName)
{
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)){
        qDebug()<<"文件无法读取";
    }
    while(!file.atEnd()){
        QByteArray bytes = file.read(256);
        tcpClientSocket->write(bytes);
        emit sendBytesCount(bytes.length());
    }

}

void TcpClientIOService::onReadReady()
{
    QByteArray bytes = tcpClientSocket->readAll();
    emit readBytes(bytes);
    emit recvBytesCount(bytes.length());
}

void TcpClientIOService::connected_slot()
{
    qDebug()<<"成功连接服务器成功";
    emit getLocalPort(tcpClientSocket->localPort());
}

void TcpClientIOService::disconnected_slot()
{
    qDebug()<<"退出断开成功";
}

void TcpClientIOService::scanLocalAvlidAddr(QComboBox *cmb)
{
    cmb->clear();
    QStringList ipAddr;

    const auto allInterfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &interface : allInterfaces)
    {
        const auto entries = interface.addressEntries();
        for (const QNetworkAddressEntry &entry : entries)
        {
            const QHostAddress &ip = entry.ip();
            // 只保留 IPv4
            if (ip.protocol() == QAbstractSocket::IPv4Protocol &&
                    !ip.isInSubnet(QHostAddress("169.254.0.0"), 16))
                ipAddr << ip.toString();
        }
    }

    cmb->addItems(ipAddr);
}
