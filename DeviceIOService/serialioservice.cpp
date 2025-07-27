#include "serialioservice.h"

SerialIOService::SerialIOService(QSerialPort *_serialPort,QObject *parent) : AbstractIOService(_serialPort, parent)
{
    serialPort = _serialPort;
}

void SerialIOService::scanAvlidSerialPort(QComboBox *cmb)
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
