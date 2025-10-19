#include "tcpserverioservice.h"

TcpServerIOService::TcpServerIOService(QObject *parent) : QObject(parent)
{
    tcpServer = new QTcpServer(this);
    connect(tcpServer, &QTcpServer::newConnection, this, &TcpServerIOService::newConnection_slot);
}

bool TcpServerIOService::isTcpServerOpen()
{
    return tcpServer->isListening();
}

bool TcpServerIOService::setLocalAddrAndPort(QString LocalAddr,quint16 port)
{
    // 先停掉上一次监听
    if (tcpServer->isListening())
        tcpServer->close();

    localPort = port;
    if(!tcpServer->listen(QHostAddress(LocalAddr),localPort))
    {
        qDebug()<<"失败服务器启动失败";
        return false;
    }else{
        qDebug()<<"成功服务器启动成功";
    }
    return true;
}

void TcpServerIOService::stopServer()
{
    for (QTcpSocket *client : qAsConst(clientList)) {
    if (client && client->state() == QAbstractSocket::ConnectedState) {
            client->disconnectFromHost();          // 优雅断开
            // 如果希望强制断开，可再调用 client->abort();
            if (client->waitForDisconnected(1000))
                qDebug() << "client disconnected gracefully";
        }
    }
    clientList.clear();
    if (tcpServer->isListening()) {
        tcpServer->close();  // 停止监听端口
        qDebug() << "TCP server stopped listening.";
    }
}

void TcpServerIOService::sendBytes(QByteArray bytes)
{
    for(int j=0; j<clientList.count(); j++)
    {
        clientList[j]->write(bytes);
    }
}

void TcpServerIOService::sendBytes(QString ipAndPort, QByteArray bytes)
{
    if (ipAndPort.toLower().contains("all"))
    {
        // 广播
        for (QTcpSocket *s : qAsConst(clientList))
            s->write(bytes);
        return;
    }

    // 单播：找到 ip:port 对应的 socket
    for (QTcpSocket *s : qAsConst(clientList))
    {
        QString key = QString("%1:%2")
                          .arg(s->peerAddress().toString())
                          .arg(s->peerPort());
        if (key == ipAndPort)
        {
            s->write(bytes);
            return;                 // 找到就结束
        }
    }

    // 可选：如果没找到，打印一条警告
    qWarning() << "sendBytes: client" << ipAndPort << "not found";
}

void TcpServerIOService::newConnection_slot()
{
    qDebug() <<"有客户端申请连接";
    QTcpSocket *s = tcpServer->nextPendingConnection();
    clientList.push_back(s);
    // 获取对端 IP 和端口
    QString ip = s->peerAddress().toString();
    quint16 port = s->peerPort();
    emit addOneConnect(ip,QString::number(port));
    connect(s, &QTcpSocket::readyRead, this, &TcpServerIOService::onReadReady);
    connect(s, &QTcpSocket::disconnected, this, [this]{
        QTcpSocket *sock = qobject_cast<QTcpSocket*>(sender());
        QString ip   = sock->peerAddress().toString();
        QString port = QString::number(sock->peerPort());
        emit delOneConnect(ip, port);
        if (sock) {
            clientList.removeOne(sock);
            sock->deleteLater();
        }
    });
}

//关于readyRead信号对应槽函数的实现
void TcpServerIOService::onReadReady()
{
    for(int i=0; i<clientList.count(); i++)
    {
        if(clientList[i]->state() == 0)
        {
            clientList.removeAt(i);     //将下标为i的客户端移除
        }
    }
    for(int i=0; i<clientList.count(); i++)
    {
        if(clientList[i]->bytesAvailable() != 0)
        {
            QByteArray msg = clientList[i]->readAll();
            emit readBytes(msg);
        }
    }
}

void TcpServerIOService::scanLocalAvlidAddr(QComboBox *cmb)
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
