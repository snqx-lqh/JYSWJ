#include "udpioservice.h"

UdpIOService::UdpIOService(QObject *parent) : QObject(parent)
{
    udpSocket = new QUdpSocket;
    connect(udpSocket, &QUdpSocket::readyRead, this, &UdpIOService::onReadReady, Qt::UniqueConnection);
}

bool UdpIOService::isUdpOpen()
{
    return udpSocket->isOpen();
}

void UdpIOService::bindAimAddressAndPort(QString aimAddr, QString aimPort, QString localAddr, QString localPort)
{
    // 1. 如果 socket 已经绑定，先关闭，防止重复绑定
    if (udpSocket->state() == QAbstractSocket::BoundState) {
        udpSocket->close();
    }

    // 2. 解析本地地址
    QHostAddress bindAddress;
    if (localAddr.isEmpty()) {
        bindAddress = QHostAddress::AnyIPv4;          // 默认监听所有 IPv4
    } else {
        bindAddress = QHostAddress(localAddr);
        if (bindAddress.isNull()) {
            qWarning() << "Invalid local address:" << localAddr;
            return;
        }
    }

    // 3. 解析端口
    bool ok = false;
    quint16 port = static_cast<quint16>(localPort.toUShort(&ok));
    if (!ok || port == 0) {
        qWarning() << "Invalid local port:" << localPort;
        return;
    }

    // 4. 绑定
    bool bindOk = udpSocket->bind(bindAddress,
                                  port,
                                  QUdpSocket::ShareAddress |
                                  QUdpSocket::ReuseAddressHint);
    if (!bindOk) {
        qWarning() << "UDP bind failed:" << udpSocket->errorString()
                   << "@" << bindAddress.toString() << ":" << port;
        return;
    }

    // 6. 保存“目标地址 / 端口”供后续 sendDatagram 使用（可选）
    m_aimHost = QHostAddress(aimAddr);
    m_aimPort = static_cast<quint16>(aimPort.toUShort());

    qDebug() << "UDP socket bound to"
             << udpSocket->localAddress().toString()
             << ":" << udpSocket->localPort();
}

void UdpIOService::sendBytes(QByteArray bytes)
{
    udpSocket->writeDatagram(bytes.data(),bytes.size(),
                             m_aimHost,m_aimPort);
}

void UdpIOService::udpClose()
{
    udpSocket->abort();
}

void UdpIOService::onReadReady()
{
    while (udpSocket->hasPendingDatagrams())
    {
        QByteArray data;
        data.resize(udpSocket->pendingDatagramSize());

        udpSocket->readDatagram(data.data(),data.size());
        emit readBytes(data);
    }
}

void UdpIOService::scanLocalAvlidAddr(QComboBox *cmb)
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
