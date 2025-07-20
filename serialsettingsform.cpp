#include "serialsettingsform.h"
#include "ui_serialsettingsform.h"

SerialSettingsForm::SerialSettingsForm(QSerialPort *serialPort, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SerialSettingsForm)
{
    ui->setupUi(this);
    this->setWindowTitle("串口设置");

    mSerialPort = serialPort;
    scanSerialPorts();
    addBaudItems();

    connect(ui->btn_OK, &QPushButton::clicked, this, &QDialog::accept);
    connect(ui->btn_Cancel, &QPushButton::clicked, this, &QDialog::reject);
    qApp->installNativeEventFilter(this);
}

SerialSettingsForm::~SerialSettingsForm()
{
    delete ui;
}

void SerialSettingsForm::scanSerialPorts()
{
    ui->cbB_Com->clear(); // 清空当前的串口列表
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        ui->cbB_Com->addItem(info.portName()); // 添加串口名称到下拉框
    }
}

void SerialSettingsForm::addBaudItems()
{
    ui->cbB_Baud->clear();
    QStringList serialBaudString;
    serialBaudString << "110" << "300" << "600" << "1200" << "2400" << "4800" << "9600"
                     << "14400" << "19200" << "38400" << "57600" << "76800" << "115200" << "128000"
                     << "230400" << "256000" << "460800" << "921600" << "1000000" << "2000000" << "3000000"
                        ;
    ui->cbB_Baud->addItems(serialBaudString);
}

bool SerialSettingsForm::nativeEventFilter(const QByteArray & eventType, void * message, long * result)
{
    MSG* msg = reinterpret_cast<MSG*>(message);
    int msgType = msg->message;
    if (msgType == WM_DEVICECHANGE) {
        PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)msg->lParam;
        switch (msg->wParam) {
        case DBT_DEVICEARRIVAL:
            if (lpdb->dbch_devicetype == DBT_DEVTYP_PORT) {
//                PDEV_BROADCAST_PORT lpdbv = (PDEV_BROADCAST_PORT)lpdb;
//                QString port = QString::fromWCharArray(lpdbv->dbcp_name);//插入的串口名
                scanSerialPorts();
            }
            break;
        case DBT_DEVICEREMOVECOMPLETE:
            if (lpdb->dbch_devicetype == DBT_DEVTYP_PORT) {
//                PDEV_BROADCAST_PORT lpdbv = (PDEV_BROADCAST_PORT)lpdb;
//                QString port = QString::fromWCharArray(lpdbv->dbcp_name);//拔出的串口名
                scanSerialPorts();
            }
            break;
        case DBT_DEVNODES_CHANGED:
            break;
        default:
            break;
        }
    }
    return QWidget::nativeEvent(eventType, message, result);
}

void SerialSettingsForm::on_btn_OK_clicked()
{
    if(ui->cbB_Com->currentText() != "")
    {
        mSerialPort->setPortName(ui->cbB_Com->currentText());
    }

    qint32 baudSet = ui->cbB_Baud->currentText().toUInt();
    mSerialPort->setBaudRate(baudSet);

    QString stopText = ui->cbB_Stop->currentText();
    if (stopText == "1")  mSerialPort->setStopBits(QSerialPort::OneStop);
    else if(stopText == "1.5")  mSerialPort->setStopBits(QSerialPort::OneAndHalfStop);
    else if(stopText == "2")  mSerialPort->setStopBits(QSerialPort::TwoStop);

    QString dataText = ui->cbB_Data->currentText();
    if (dataText == "8")  mSerialPort->setDataBits(QSerialPort::Data8);
    else if(dataText == "7")  mSerialPort->setBaudRate(QSerialPort::Data7);
    else if(dataText == "6")  mSerialPort->setBaudRate(QSerialPort::Data6);
    else if(dataText == "5")  mSerialPort->setBaudRate(QSerialPort::Data5);

    QString parityText = ui->cbB_Parity->currentText();
    if (parityText == "None")  mSerialPort->setParity(QSerialPort::NoParity);
    else if(parityText == "Odd")  mSerialPort->setParity(QSerialPort::OddParity);
    else if(parityText == "Even")  mSerialPort->setParity(QSerialPort::EvenParity);

    QString flowText = ui->cbB_Flow->currentText();
    if (flowText == "NoFlow")  mSerialPort->setFlowControl(QSerialPort::NoFlowControl);
    else if(flowText == "Hardware")  mSerialPort->setFlowControl(QSerialPort::HardwareControl);
    else if(flowText == "Software")  mSerialPort->setFlowControl(QSerialPort::SoftwareControl);
}


void SerialSettingsForm::on_btn_Cancel_clicked()
{

}

void SerialSettingsForm::onUpdateSerialInfo()
{
    ui->cbB_Com->setCurrentText(mSerialPort->portName());
    ui->cbB_Baud->setCurrentText(QString::number(mSerialPort->baudRate()));

    QSerialPort::DataBits dataBits = mSerialPort->dataBits();
    if (dataBits == QSerialPort::Data8)      ui->cbB_Data->setCurrentText("8");
    else if(dataBits == QSerialPort::Data7)  ui->cbB_Data->setCurrentText("7");
    else if(dataBits == QSerialPort::Data6)  ui->cbB_Data->setCurrentText("6");
    else if(dataBits == QSerialPort::Data5)  ui->cbB_Data->setCurrentText("5");

    QSerialPort::StopBits stopBits = mSerialPort->stopBits();
    if (stopBits == QSerialPort::OneStop) ui->cbB_Stop->setCurrentText("1");
    else if(stopBits == QSerialPort::OneAndHalfStop)  ui->cbB_Stop->setCurrentText("1.5");
    else if(stopBits == QSerialPort::TwoStop)  ui->cbB_Stop->setCurrentText("2");

    QSerialPort::Parity parity = mSerialPort->parity();
    if (parity == QSerialPort::NoParity)       ui->cbB_Parity->setCurrentText("None");
    else if(parity == QSerialPort::OddParity)  ui->cbB_Parity->setCurrentText("Odd");
    else if(parity == QSerialPort::EvenParity) ui->cbB_Parity->setCurrentText("Even");

    QSerialPort::FlowControl flowControl = mSerialPort->flowControl();

    if (flowControl == QSerialPort::NoFlowControl)  ui->cbB_Flow->setCurrentText("NoFlow");
    else if(flowControl == QSerialPort::HardwareControl)  ui->cbB_Flow->setCurrentText("Hardware");
    else if(flowControl == QSerialPort::SoftwareControl)  ui->cbB_Flow->setCurrentText("Software");
}

