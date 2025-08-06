#ifndef TCPSERVERIOSERVICE_H
#define TCPSERVERIOSERVICE_H

#include <QObject>
#include <QtNetwork>
#include <QList>
#include <QMessageBox>
#include <QDebug>
#include <QComboBox>
#include <QTcpServer>

class TcpServerIOService : public QObject
{
    Q_OBJECT
public:
    explicit TcpServerIOService(QObject *parent = nullptr);

    void setLocalAddrAndPort(QString LocalAddr,quint16 port);
    void sendBytes(QByteArray);
    void sendBytes(QString,QByteArray);

    void scanLocalAvlidAddr(QComboBox *cmb);
    void stopServer();
signals:
    void readBytes(QByteArray);

    void addOneConnect(QString ip,QString port);
    void delOneConnect(QString ip,QString port);

public slots:
    void newConnection_slot();
    void onReadReady();

private:
    quint16 localPort;
    QTcpServer *tcpServer;
    //定义客户端指针链表容器
    QList<QTcpSocket *> clientList;
};

#endif // TCPSERVERIOSERVICE_H
