#include "serialioservice.h"

SerialIOService::SerialIOService(QObject *parent) : QObject(parent)
{
    serialPort = new QSerialPort;
    connect(serialPort,&QSerialPort::readyRead,this,&SerialIOService::onReadReady);
}

bool SerialIOService::openSerial()
{
    int connect = serialPort->open(QSerialPort::ReadWrite);
    if(connect){
        qDebug() << "串口打开成功";
    }else{
        qDebug() << "串口打开失败";
        QMessageBox msg;
        msg.setText("串口打开失败");
        msg.show();
        msg.exec();
    }
    return connect;
}

void SerialIOService::closeSerial()
{
    serialPort->close();
}

bool SerialIOService::isSerialOpen()
{
    return serialPort->isOpen();
}

void SerialIOService::sendBytes(QByteArray bytes)
{
    serialPort->write(bytes);
    emit sendBytesCount(bytes.length());
}

void SerialIOService::sendFile(QString fileName)
{
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)){
        qDebug()<<"文件无法读取";
    }
    while(!file.atEnd()){
        QByteArray bytes = file.read(256);
        serialPort->write(bytes);
        emit sendBytesCount(bytes.length());
    }

}

void SerialIOService::onReadReady()
{
    QByteArray bytes = serialPort->readAll();
    emit readBytes(bytes);
    emit recvBytesCount(bytes.length());
}

void SerialIOService::scanAvailableSerialPort(QComboBox *cmb)
{
    cmb->clear();
    foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts()){
        cmb->addItem(info.portName()/*+" "+info.description()*/);
    }
}

void SerialIOService::addBaudItems(QComboBox *cmb)
{
    cmb->clear();
    QStringList serialBaudString;
    serialBaudString << "110" << "300" << "600" << "1200" << "2400" << "4800" << "9600"
                     << "14400" << "19200" << "38400" << "57600" << "76800" << "115200" << "128000"
                     << "230400" << "256000" << "460800" << "921600" << "1000000" << "2000000" << "3000000"
                        ;
    cmb->addItems(serialBaudString);
}

void SerialIOService::setPortName(QString portName)
{
    serialPort->setPortName(portName);
}

void SerialIOService::setBaudRate(QString baudRate)
{
    serialPort->setBaudRate(baudRate.toUInt());
}

void SerialIOService::setStopBits(QString stopBits)
{
    uint8_t openFlag = 0;
    if (serialPort->isOpen()){
        openFlag = 1;
        serialPort->close();
    }
    if (stopBits == "1")  serialPort->setStopBits(QSerialPort::OneStop);
    else if(stopBits == "1.5")  serialPort->setStopBits(QSerialPort::OneAndHalfStop);
    else if(stopBits == "2")  serialPort->setStopBits(QSerialPort::TwoStop);

    if(openFlag == 1) serialPort->open(QSerialPort::ReadWrite);
}

void SerialIOService::setDataBits(QString dataBits)
{
    uint8_t openFlag = 0;
    if (serialPort->isOpen()){
        openFlag = 1;
        serialPort->close();
    }
    if (dataBits == "8")  serialPort->setDataBits(QSerialPort::Data8);
    else if(dataBits == "7")  serialPort->setBaudRate(QSerialPort::Data7);
    else if(dataBits == "6")  serialPort->setBaudRate(QSerialPort::Data6);
    else if(dataBits == "5")  serialPort->setBaudRate(QSerialPort::Data5);
    if(openFlag == 1) serialPort->open(QSerialPort::ReadWrite);
}

void SerialIOService::setParity(QString parity)
{
    uint8_t openFlag = 0;
    if (serialPort->isOpen()){
        openFlag = 1;
        serialPort->close();
    }
    if (parity == "None")  serialPort->setParity(QSerialPort::NoParity);
    else if(parity == "Odd")  serialPort->setParity(QSerialPort::OddParity);
    else if(parity == "Even")  serialPort->setParity(QSerialPort::EvenParity);
    if(openFlag == 1) serialPort->open(QSerialPort::ReadWrite);
}

QString SerialIOService::getConnectInfo()
{
    QString infoStr = "";
    infoStr += serialPort->portName() + ",";
    infoStr += QString::number(serialPort->baudRate()) + ",";

    QSerialPort::DataBits dataBits = serialPort->dataBits();
    if (dataBits == QSerialPort::Data8)      infoStr += "8,";
    else if(dataBits == QSerialPort::Data7)  infoStr += "7,";
    else if(dataBits == QSerialPort::Data6)  infoStr += "6,";
    else if(dataBits == QSerialPort::Data5)  infoStr += "5,";

    QSerialPort::StopBits stopBits = serialPort->stopBits();
    if (stopBits == QSerialPort::OneStop) infoStr += "1,";
    else if(stopBits == QSerialPort::OneAndHalfStop)  infoStr += "1.5,";
    else if(stopBits == QSerialPort::TwoStop)  infoStr += "2,";

    QSerialPort::Parity parity = serialPort->parity();
    if (parity == QSerialPort::NoParity)       infoStr += "None";
    else if(parity == QSerialPort::OddParity)  infoStr += "Odd";
    else if(parity == QSerialPort::EvenParity) infoStr += "Even";

//    QSerialPort::FlowControl flowControl = serialPort->flowControl();

//    if (flowControl == QSerialPort::NoFlowControl)  infoStr += "NoFlow";
//    else if(flowControl == QSerialPort::HardwareControl)  infoStr += "Hardware";
//    else if(flowControl == QSerialPort::SoftwareControl)  infoStr += "Software";

    return infoStr;
}
