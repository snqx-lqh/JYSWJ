#ifndef ABSTRACTIOSERVICE_H
#define ABSTRACTIOSERVICE_H

#include <QObject>
#include <QIODevice>
#include <QFile>
#include <QDebug>

class AbstractIOService : public QObject
{
    Q_OBJECT
public:
    explicit AbstractIOService(QIODevice *_ioDevice, QObject *parent = nullptr);

    QIODevice * getIODevice();

    void sendBytes(QByteArray);
    void sendFile(QString);

public slots:
    void onReadReady();

signals:
    void readBytes(QByteArray);
    void sendBytesCount(uint32_t);
    void recvBytesCount(uint32_t);

private:
    QIODevice *ioDevice;

};

#endif // ABSTRACTIOSERVICE_H
