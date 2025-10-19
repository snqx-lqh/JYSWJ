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
    if (port == 0 || port > 65535) {
        emit connectionResult(false, tr("端口无效: %1").arg(port));
        return false;
    }

    QHostAddress addr;
    if (!addr.setAddress(hostName)) {
        QRegularExpression re(R"(^[\w.-]+$)");
        if (!re.match(hostName).hasMatch()) {
            emit connectionResult(false, tr("无效的主机名格式: %1").arg(hostName));
            return false;
        }
    }

    if (tcpClientSocket->state() != QAbstractSocket::UnconnectedState) {
        tcpClientSocket->abort();
    }

    tcpClientSocket->connectToHost(hostName, port);

    // 🔹等待连接 3 秒（可异步，也可同步）
    if (!tcpClientSocket->waitForConnected(3000)) {
        emit connectionResult(false, tr("连接超时或失败: %1").arg(tcpClientSocket->errorString()));
        return false;
    }

    return true;
}

void TcpClientIOService::disconnectServer()
{
    if (tcpClientSocket->state() != QAbstractSocket::UnconnectedState)
        tcpClientSocket->disconnectFromHost();
}

void TcpClientIOService::sendBytes(QByteArray bytes)
{
    if (!tcpClientSocket->isOpen())
        return;
    tcpClientSocket->write(bytes);
    emit sendBytesCount(bytes.length());
}

void TcpClientIOService::sendFile(QString fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "文件无法读取:" << fileName;
        return;
    }

    while (!file.atEnd()) {
        QByteArray bytes = file.read(512);
        tcpClientSocket->write(bytes);
        emit sendBytesCount(bytes.length());
    }

    file.close();

}

void TcpClientIOService::onReadReady()
{
    QByteArray bytes = tcpClientSocket->readAll();
    emit readBytes(bytes);
    emit recvBytesCount(bytes.length());
}

void TcpClientIOService::connected_slot()
{
    qDebug() << "成功连接服务器:" << tcpClientSocket->peerAddress().toString();
    emit getLocalPort(tcpClientSocket->localPort());
    emit connectionResult(true, "连接成功");
}

void TcpClientIOService::disconnected_slot()
{
    qDebug() << "与服务器断开连接";
    emit connectionResult(false, "已断开连接");
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
