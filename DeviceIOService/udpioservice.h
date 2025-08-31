#ifndef UDPIOSERVICE_H
#define UDPIOSERVICE_H

#include <QtNetwork>
#include <QDebug>
#include <QComboBox>

class UdpIOService : public QObject
{
    Q_OBJECT
public:
    explicit UdpIOService(QObject *parent = nullptr);

    bool isUdpOpen           (void);

    void scanLocalAvlidAddr(QComboBox* cmb);
    void bindAimAddressAndPort(QString aimAddr,QString aimPort,QString localAddr,QString localPort);
    void sendBytes(QByteArray bytes);
    void udpClose();

signals:
    void readBytes(QByteArray);

public slots:
    void onReadReady();

private:
    QUdpSocket *udpSocket;
    QHostAddress  m_aimHost;   // 保存目标 IP
    quint16       m_aimPort;   // 保存目标端口
};

#endif // UDPIOSERVICE_H
