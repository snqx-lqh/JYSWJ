#ifndef SERIALIOSERVICE_H
#define SERIALIOSERVICE_H

#include <QObject>
#include <QComboBox>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QFile>
#include <QDebug>
#include <QMessageBox>

class SerialIOService : public QObject
{
    Q_OBJECT
public:
    explicit SerialIOService(QObject *parent = nullptr);

    bool openSerial();
    void closeSerial();
    bool isSerialOpen();

    void sendBytes(QByteArray);
    void sendFile(QString);

    void scanAvailableSerialPort(QComboBox*);
    void addBaudItems(QComboBox*);

    void setPortName(QString);
    void setBaudRate(QString);
    void setStopBits(QString);
    void setDataBits(QString);
    void setParity(QString);
    QString getConnectInfo();

public slots:
    void onReadReady();

signals:
    void readBytes(QByteArray);
    void sendBytesCount(uint32_t);
    void recvBytesCount(uint32_t);

private:
    QSerialPort *serialPort;
};

#endif // SERIALIOSERVICE_H
