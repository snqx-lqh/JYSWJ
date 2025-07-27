#include "abstractioservice.h"

AbstractIOService::AbstractIOService(QIODevice *_ioDevice,QObject *parent) : QObject(parent)
{
    ioDevice = _ioDevice;
    connect(ioDevice,&QIODevice::readyRead,this,&AbstractIOService::onReadReady);
}


QIODevice *AbstractIOService::getIODevice()
{
    return ioDevice;
}

void AbstractIOService::sendBytes(QByteArray bytes)
{
    ioDevice->write(bytes);
    emit sendBytesCount(bytes.length());
}

void AbstractIOService::sendFile(QString fileName)
{
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)){
        qDebug()<<"文件无法读取";
    }
    while(!file.atEnd()){
        QByteArray bytes = file.read(256);
        ioDevice->write(bytes);
        emit sendBytesCount(bytes.length());
    }

}

void AbstractIOService::onReadReady()
{
    QByteArray bytes = ioDevice->readAll();
    emit readBytes(bytes);
    emit recvBytesCount(bytes.length());
}
