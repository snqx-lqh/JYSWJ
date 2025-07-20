#include "portsettingsform.h"
#include "ui_portsettingsform.h"

PortSettingsForm::PortSettingsForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PortSettingsForm)
{
    ui->setupUi(this);
    serialPort = new QSerialPort;

    scanVaildPort();
    addBaudItems();

    serialSettingsForm = new SerialSettingsForm(serialPort);

    connect(serialPort,&QIODevice::readyRead,this,&PortSettingsForm::onReadBytes);

}

PortSettingsForm::~PortSettingsForm()
{
    delete ui;
}

void PortSettingsForm::scanVaildPort()
{
    ui->cmb_Port->clear();
    foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts()){
        ui->cmb_Port->addItem(info.portName());
    }
}

void PortSettingsForm::addBaudItems()
{
    ui->cmb_Baud->clear();
    QStringList serialBaudString;
    serialBaudString << "110" << "300" << "600" << "1200" << "2400" << "4800" << "9600"
                     << "14400" << "19200" << "38400" << "57600" << "76800" << "115200" << "128000"
                     << "230400" << "256000" << "460800" << "921600" << "1000000" << "2000000" << "3000000"
                        ;
    ui->cmb_Baud->addItems(serialBaudString);
}

void PortSettingsForm::saveOptions(QSettings *settings)
{
    settings->beginGroup("portSetting");
    settings->setValue("SerialPort", ui->cmb_Port->currentText());
    settings->setValue("SerialBaud", ui->cmb_Baud->currentText());
    settings->setValue("cb_sendNewLine_state",ui->cb_SendNewLine->isChecked());
    settings->setValue("cb_sendHex_state",ui->cb_SendHex->isChecked());
    settings->setValue("cb_AddTime_state",ui->cb_AddTime->isChecked());
    settings->setValue("cb_HexShow_state",ui->cb_HexShow->isChecked());
    settings->setValue("cb_ShowSend_state",ui->cb_ShowSend->isChecked());

    settings->endGroup();
}

void PortSettingsForm::loadOptions(QSettings *settings)
{
    settings->beginGroup("portSetting");
    auto serialPortName = settings->value("SerialPort","");
    auto serialBaud     = settings->value("SerialBaud","115200");
    auto cb_sendNewLine_state = settings->value("cb_sendNewLine_state","false");
    auto cb_sendHex_state = settings->value("cb_sendHex_state","false");
    auto cb_AddTime_state = settings->value("cb_AddTime_state","false");
    auto cb_HexShow_state = settings->value("cb_HexShow_state","false");
    auto cb_ShowSend_state = settings->value("cb_ShowSend_state","false");

    ui->cmb_Port->setCurrentText(serialPortName.toString());
    ui->cmb_Baud->setCurrentText(serialBaud.toString());
    ui->cb_SendNewLine->setChecked(cb_sendNewLine_state.toBool());
    ui->cb_SendHex->setChecked(cb_sendHex_state.toBool());
    ui->cb_AddTime->setChecked(cb_AddTime_state.toBool());
    ui->cb_HexShow->setChecked(cb_HexShow_state.toBool());
    ui->cb_ShowSend->setChecked(cb_ShowSend_state.toBool());

    settings->endGroup();
}

QString PortSettingsForm::getConnectInfo()
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

    if (flowControl == QSerialPort::NoFlowControl)  infoStr += "NoFlow";
    else if(flowControl == QSerialPort::HardwareControl)  infoStr += "Hardware";
    else if(flowControl == QSerialPort::SoftwareControl)  infoStr += "Software";

    return infoStr;
}

void PortSettingsForm::onSendBytes(QByteArray bytes)
{
    serialPort->write(bytes);
}


void PortSettingsForm::on_btn_SerialSet_clicked()
{
    serialSettingsForm->show();
    serialSettingsForm->onUpdateSerialInfo();
    int result =  serialSettingsForm->exec();
    if(result == QDialog::Accepted){
        ui->cmb_Port->setCurrentText(serialPort->portName());
        ui->cmb_Baud->setCurrentText(QString::number(serialPort->baudRate()));
        emit sendConnectInfo(getConnectInfo());
    }else {
        ;
    }
}

void PortSettingsForm::on_btn_OpenSerial_clicked()
{
    if(serialPort->isOpen()){
        serialPort->close();
        ui->cmb_Port->setEnabled(true);
        ui->cmb_Baud->setEnabled(true);
        ui->btn_OpenSerial->setText("打开串口");
    }else{
        serialPort->setPortName(ui->cmb_Port->currentText());
        serialPort->setBaudRate(ui->cmb_Baud->currentText().toUInt());

        int connect = serialPort->open(QSerialPort::ReadWrite);
        if(connect){
            qDebug() << "串口打开成功";
        }else{
            qDebug() << "串口打开失败";
            return;
        }
        ui->cmb_Port->setEnabled(false);
        ui->cmb_Baud->setEnabled(false);
        ui->btn_OpenSerial->setText("关闭串口");
    }
}

void PortSettingsForm::onSendFile(QString fileName)
{
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)){
        qDebug()<<"文件无法读取";
    }
    while(!file.atEnd()){
        QByteArray bytes = file.read(256);
        serialPort->write(bytes);
        serialPort->waitForBytesWritten();
    }
}


void PortSettingsForm::onReadBytes()
{
    QByteArray bytes = serialPort->readAll();
    if(receiveFile.isOpen()){
        //receiveTextStream<<bytes;
        receiveFile.write(bytes);
    }
    emit readBytes(bytes);
}

void PortSettingsForm::on_btn_ClearSend_clicked()
{
    emit clearSend();
}

void PortSettingsForm::on_btn_ClearRecv_clicked()
{
    emit clearRecv();
}



void PortSettingsForm::on_cmb_Port_currentTextChanged(const QString &arg1)
{
    serialPort->setPortName(arg1);
    emit sendConnectInfo(getConnectInfo());
}


void PortSettingsForm::on_cmb_Baud_currentTextChanged(const QString &arg1)
{
    serialPort->setBaudRate(arg1.toUInt());
    emit sendConnectInfo(getConnectInfo());
}


void PortSettingsForm::on_cb_AddTime_stateChanged(int arg1)
{
    Q_UNUSED(arg1);
    bool state = ui->cb_AddTime->isChecked();
    emit sendCheckTime(state);
}


void PortSettingsForm::on_cb_HexShow_stateChanged(int arg1)
{
    Q_UNUSED(arg1);
    bool state = ui->cb_HexShow->isChecked();
    emit sendHexShow(state);
}


void PortSettingsForm::on_cb_ShowSend_stateChanged(int arg1)
{
    Q_UNUSED(arg1);
    bool state = ui->cb_ShowSend->isChecked();
    emit sendShowSend(state);
}


void PortSettingsForm::on_cb_SendHex_stateChanged(int arg1)
{
    Q_UNUSED(arg1);
    bool state = ui->cb_SendHex->isChecked();
    emit sendSendHex(state);
}


void PortSettingsForm::on_cb_RecvToFile_clicked(bool checked)
{
    if(checked == true){
        QString filePath = QDir::currentPath() + "/receiveData_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".txt";

        receiveFile.setFileName(filePath);
        if(receiveFile.open(QIODevice::WriteOnly | QIODevice::Text)){
            receiveTextStream.setDevice(&receiveFile);
        }

        QMessageBox msg;
        msg.setText("文件将保存在"+filePath);
        msg.show();
        msg.exec();
    }else{
        if(receiveFile.isOpen()){
            receiveFile.close();
        }
    }
}


void PortSettingsForm::on_btn_saveData_clicked()
{
    emit saveRecvEditData();
}

