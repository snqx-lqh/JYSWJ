#ifndef SERIALIOSERVICE_H
#define SERIALIOSERVICE_H

#include "abstractioservice.h"
#include <QObject>
#include <QComboBox>
#include <QSerialPort>
#include <QSerialPortInfo>

class SerialIOService : public AbstractIOService
{
    Q_OBJECT
public:
    explicit SerialIOService(QSerialPort *_serialPort, QObject *parent = nullptr);
    void scanAvlidSerialPort(QComboBox*);
    void addBaudItems(QComboBox*);
    QString getConnectInfo();

private:
    QSerialPort *serialPort;
};

#endif // SERIALIOSERVICE_H
