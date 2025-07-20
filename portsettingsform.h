#ifndef PORTSETTINGSFORM_H
#define PORTSETTINGSFORM_H

#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QList>
#include <QSettings>
#include <QDebug>
#include <QFileDialog>
#include <QDir>
#include <QDateTime>
#include "serialsettingsform.h"
#include <QMessageBox>
#include <QFile>

namespace Ui {
class PortSettingsForm;
}

class PortSettingsForm : public QWidget
{
    Q_OBJECT

public:
    explicit PortSettingsForm(QWidget *parent = nullptr);
    ~PortSettingsForm();

    void scanVaildPort();
    void addBaudItems();
    void saveOptions(QSettings *);
    void loadOptions(QSettings *);
    QString getConnectInfo();

signals:
    void readBytes(QByteArray);
    void clearSend();
    void clearRecv();
    void sendConnectInfo(QString);
    void sendSendHex(bool);
    void sendCheckTime(bool);
    void sendHexShow(bool);
    void sendShowSend(bool);
    void saveRecvEditData();

public slots:
    void onSendBytes(QByteArray);
    void onSendFile(QString);
    void onReadBytes( );

private slots:
    void on_btn_SerialSet_clicked();

    void on_btn_OpenSerial_clicked();

    void on_btn_ClearSend_clicked();

    void on_btn_ClearRecv_clicked();

    void on_cmb_Port_currentTextChanged(const QString &arg1);

    void on_cmb_Baud_currentTextChanged(const QString &arg1);

    void on_cb_AddTime_stateChanged(int arg1);

    void on_cb_HexShow_stateChanged(int arg1);

    void on_cb_ShowSend_stateChanged(int arg1);

    void on_cb_SendHex_stateChanged(int arg1);

    void on_cb_RecvToFile_clicked(bool checked);

    void on_btn_saveData_clicked();

private:
    Ui::PortSettingsForm *ui;

    QSerialPort *serialPort;

    SerialSettingsForm *serialSettingsForm;

    QFile         receiveFile;
    QTextStream   receiveTextStream;
};

#endif // PORTSETTINGSFORM_H
