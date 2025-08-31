#ifndef TCPCLIENTIOSERVICE_H
#define TCPCLIENTIOSERVICE_H

#include <QObject>
#include <QtNetwork>
#include <QDebug>
#include <QFile>
#include <QComboBox>

class TcpClientIOService : public QObject
{
    Q_OBJECT
public:
    explicit TcpClientIOService(QObject *parent = nullptr);

    bool isTcpClientOpen           (void);

    void sendBytes(QByteArray);
    void sendFile(QString);

    bool connectServer(QString hostName,quint32 port);
    void disconnectServer();
    void scanLocalAvlidAddr(QComboBox *cmb);

public slots:
    void onReadReady();
    void connected_slot();      //自定义处理信号的槽函数
    void disconnected_slot();   //自定义处理disconnected信号的槽函数

signals:
    void readBytes(QByteArray);
    void sendBytesCount(uint32_t);
    void recvBytesCount(uint32_t);

    void getLocalPort(quint16);
private:
    //定义一个客户端指针
    QTcpSocket *tcpClientSocket;
};

#endif // TCPCLIENTIOSERVICE_H
