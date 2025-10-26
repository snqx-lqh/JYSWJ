#include "serialioservice.h"



SerialIOService::SerialIOService(QObject *parent) : QObject(parent)
{
    serialPort = new QSerialPort;
    connect(serialPort,&QSerialPort::readyRead,this,&SerialIOService::onReadReady);
}

SerialIOService::~SerialIOService()
{
    delete serialPort;
}

bool SerialIOService::openSerial()
{
    int connect = serialPort->open(QSerialPort::ReadWrite);
    if(connect){
        qDebug() << "串口打开成功";
    }else{
        qDebug() << "串口打开失败";
        QMessageBox::warning(nullptr,"串口打开提示","串口打开失败！！！");
    }
    return connect;
}

void SerialIOService::closeSerial()
{
    if(serialPort && isSerialOpen()){
        serialPort->close();
    }
}

bool SerialIOService::isSerialOpen()
{
    return serialPort->isOpen();
}

void SerialIOService::sendBytes(QByteArray bytes)
{
    if(!serialPort->isOpen()){
        QMessageBox::warning(nullptr,"提示","串口未打开");
        return;
    }
    serialPort->write(bytes);
    emit sendBytesCount(bytes.length());
}

void SerialIOService::sendFile(QString filePath)
{
    if(!isSerialOpen()){
        QMessageBox::warning(nullptr,"提示","串口未打开");
        return;
    }

    file.setFileName(filePath);
    fileLength = file.size();
    fileReadLength = 0;
    if(!file.open(QIODevice::ReadOnly)){
        qDebug() << "文件无法读取";
        return;
    }

    connect(serialPort, &QSerialPort::bytesWritten,
                this, &SerialIOService::handleBytesWritten, Qt::UniqueConnection);

    // 先发送一块
    writeNextChunk();
}

void SerialIOService::writeNextChunk()
{
    if(!file.atEnd()){
        QByteArray bytes = file.read(256);
        serialPort->write(bytes);  // 这里不阻塞
        emit sendBytesCount(bytes.length());
        fileReadLength += bytes.size();
        if(progressBar)
            progressBar->setValue(fileReadLength*100/fileLength);
    } else {
        file.close();
        disconnect(serialPort, &QSerialPort::bytesWritten,
                    this, &SerialIOService::handleBytesWritten);
        QMessageBox::information(nullptr,"文件发送提示","文件发送完成");
        if(progressBar)
            progressBar->setValue(0);
        qDebug() << "文件发送完成";
    }
}

void SerialIOService::handleBytesWritten(qint64 bytes)
{
    Q_UNUSED(bytes);
    writeNextChunk();  // 每发完一块，再发下一块
}

QByteArray SerialIOService::safeRead(QSerialPort *dev, qint64 maxSize)
{
    if (!dev || !dev->isOpen()) {
        qWarning() << "Device not open";
        return {};
    }

    const qint64 avail = dev->bytesAvailable();
    if (avail <= 0) {
        return {};
    }

    const qint64 toRead = qMin(avail, maxSize);
    QByteArray data(toRead, Qt::Uninitialized);
    const qint64 actuallyRead = dev->read(data.data(), toRead);

    if (actuallyRead < 0) {
        qWarning() << "Read error:" << dev->errorString();
        return {};
    }

    data.resize(actuallyRead);   // 防止少读
    return data;
}

void SerialIOService::onReadReady()
{
    QByteArray bytes = safeRead(serialPort, 16 * 1024);
    emit readBytes(bytes);
    emit recvBytesCount(bytes.length());
}

void SerialIOService::scanAvailableSerialPort(QComboBox *cmb)
{
    cmb->clear();
    foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts()){
        cmb->addItem(info.portName()+" "+info.description());
    }
}

void SerialIOService::addCommonBaudItem(QComboBox *cmb)
{
    cmb->clear();
    if(serialBaudList.isEmpty()){
        serialBaudList << "110"    << "300"    << "600"    << "1200"   << "2400"    << "4800"    << "9600"
                       << "14400"  << "19200"  << "38400"  << "57600"  << "76800"   << "115200"  << "128000"
                       << "230400" << "256000" << "460800" << "921600" << "1000000" << "2000000" << "3000000";
    }
    cmb->addItems(serialBaudList);
}

void SerialIOService::addCustomBaudItem(QComboBox *cmb, QString serialBaud, bool sortFlag)
{
    serialBaudList<<serialBaud;
    cmb->addItem(serialBaud);

    if(sortFlag){
        QList<qint32> list_baud;
        for(int i=0;i<cmb->count();i++){
            list_baud<<cmb->itemText(i).toUInt();
        }
        std::sort(list_baud.begin(),list_baud.end());
        QStringList strs;
        strs.reserve(list_baud.size());
        for(qint32 n:list_baud)
            strs<<QString::number(n);
        cmb->clear();
        cmb->addItems(strs);
    }
    cmb->setCurrentText(serialBaud);
}

void SerialIOService::delCustomBaudItem(QComboBox *cmb, int index, bool sortFlag)
{
    serialBaudList.removeAt(index);
    cmb->removeItem(index);

    if(sortFlag){
        QList<qint32> list_baud;
        for(int i=0;i<cmb->count();i++){
            list_baud<<cmb->itemText(i).toUInt();
        }
        std::sort(list_baud.begin(),list_baud.end());
        QStringList strs;
        strs.reserve(list_baud.size());
        for(qint32 n:list_baud)
            strs<<QString::number(n);
        cmb->clear();
        cmb->addItems(strs);
    }
}

void SerialIOService::updateBaudItem(QComboBox *cmb)
{
    QString serialBaud = getSerialBaudRate();
    cmb->clear();
    cmb->addItems(serialBaudList);
    cmb->setCurrentText(serialBaud);
}

void SerialIOService::loadSettings()
{
    QDir dir(QCoreApplication::applicationDirPath());
    QSettings settings(dir.filePath("settings.ini"),QSettings::IniFormat);

    settings.beginGroup("serialWork");
    serialBaudList = settings.value("serialBaudList").toStringList();
    settings.endGroup();
}

void SerialIOService::saveSettings()
{
    QDir dir(QCoreApplication::applicationDirPath());
    QSettings settings(dir.filePath("settings.ini"),QSettings::IniFormat);

    settings.beginGroup("serialWork");
    settings.setValue("serialBaudList", serialBaudList);
    settings.endGroup();
}

void SerialIOService::setSerialPortName(QString portName)
{
    serialPort->setPortName(portName);
}

void SerialIOService::setSerialBaudRate(QString baudRate)
{
    serialPort->setBaudRate(baudRate.toUInt());
}

void SerialIOService::setSerialStopBits(QString stopBits)
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

void SerialIOService::setSerialDataBits(QString dataBits)
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

void SerialIOService::setSerialParity(QString parity)
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

QString SerialIOService::getSerialPortName()
{
    return serialPort->portName();
}

QString SerialIOService::getSerialBaudRate()
{
    return QString::number(serialPort->baudRate());
}

QString SerialIOService::getSerialStopBits()
{
    QString dataBitsStr = "";
    QSerialPort::DataBits dataBits = serialPort->dataBits();
    if (dataBits == QSerialPort::Data8)      dataBitsStr = "8";
    else if(dataBits == QSerialPort::Data7)  dataBitsStr = "7";
    else if(dataBits == QSerialPort::Data6)  dataBitsStr = "6";
    else if(dataBits == QSerialPort::Data5)  dataBitsStr = "5";
    return dataBitsStr;
}

QString SerialIOService::getSerialDataBits()
{
    QString stopBitsStr = "";
    QSerialPort::StopBits stopBits = serialPort->stopBits();
    if (stopBits == QSerialPort::OneStop) stopBitsStr = "1";
    else if(stopBits == QSerialPort::OneAndHalfStop)  stopBitsStr = "1.5";
    else if(stopBits == QSerialPort::TwoStop) stopBitsStr = "2";
    return stopBitsStr;
}

QString SerialIOService::getSerialParity()
{
    QString parityStr = "";
    QSerialPort::Parity parity = serialPort->parity();
    if (parity == QSerialPort::NoParity)       parityStr = "None";
    else if(parity == QSerialPort::OddParity)  parityStr = "Odd";
    else if(parity == QSerialPort::EvenParity) parityStr = "Even";
    return parityStr;
}

QString SerialIOService::getSerialFlowControl()
{
    QString flowControlStr = "";
    QSerialPort::FlowControl flowControl = serialPort->flowControl();
    if (flowControl == QSerialPort::NoFlowControl)  flowControlStr = "None";
    else if(flowControl == QSerialPort::HardwareControl)  flowControlStr = "Hardware";
    else if(flowControl == QSerialPort::SoftwareControl)  flowControlStr = "Software";
    return flowControlStr;
}

QString SerialIOService::getSerialConnectInfo()
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
    if (parity == QSerialPort::NoParity)       infoStr += "None,";
    else if(parity == QSerialPort::OddParity)  infoStr += "Odd,";
    else if(parity == QSerialPort::EvenParity) infoStr += "Even,";

    QSerialPort::FlowControl flowControl = serialPort->flowControl();
    if (flowControl == QSerialPort::NoFlowControl)  infoStr += "None";
    else if(flowControl == QSerialPort::HardwareControl)  infoStr += "Hardware";
    else if(flowControl == QSerialPort::SoftwareControl)  infoStr += "Software";

    return infoStr;
}

QSerialPort *SerialIOService::getSerial()
{
    return serialPort;
}

void SerialIOService::setProgressBar(QProgressBar *_progressBar)
{
    progressBar = _progressBar;
}
