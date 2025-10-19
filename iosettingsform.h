#ifndef IOSETTINGSFORM_H
#define IOSETTINGSFORM_H

#include <QWidget>
#include <QDir>
#include <QSettings>
#include <QComboBox>
#include <QStyledItemDelegate>
#include <QAbstractNativeEventFilter>
#include <Windows.h>
#include <dbt.h>

#include "serialioservice.h"
#include "udpioservice.h"
#include "tcpclientioservice.h"
#include "tcpserverioservice.h"
#include "common.h"

namespace Ui {
class IOSettingsForm;
}

class IOSettingsForm : public QWidget,QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    explicit IOSettingsForm(QWidget *parent = nullptr);
    ~IOSettingsForm();

    enum ConnectMode{
        Serial       = 0,
        Udp          = 1,
        TcpServer    = 2,
        TcpClient    = 3,
        None         = -1
    };

    void loadSettings();
    void saveSettings();
    void setProgressBar(QProgressBar *_progressBar);
    void stateInit();
    bool nativeEventFilter(const QByteArray & eventType, void * message, long * result);
public slots:
    void onReadBytes(QByteArray bytes);
    void onSendBytes(QByteArray bytes);
    void onProtocolSendBytes(QByteArray bytes);
    void onSendFile(QString filePath);
    void onStateChange(STATE_CHANGE_TYPE_T type,int state);
    void onSendCountChanged(uint32_t count);
    void onRecvCountChanged(uint32_t count);

signals:
    void readBytes(QByteArray bytes);
    void stateChange(STATE_CHANGE_TYPE_T type,int state);
    void appendSendData(QByteArray bytes);
    void connectInfo(QString connectInfo);

private slots:
    void on_comboBox_PortName_currentTextChanged(const QString &arg1);

    void on_comboBox_BaudRate_currentTextChanged(const QString &arg1);

    void on_comboBox_StopBits_currentTextChanged(const QString &arg1);

    void on_comboBox_DataBits_currentTextChanged(const QString &arg1);

    void on_comboBox_Parity_currentTextChanged(const QString &arg1);

    void on_checkBox_HexShow_stateChanged(int arg1);

    void on_checkBox_DateTime_stateChanged(int arg1);

    void on_checkBox_SendNewLine_stateChanged(int arg1);

    void on_pushButton_ClearRecv_clicked();

    void on_checkBox_RecvToFile_stateChanged(int arg1);

    void on_comboBox_Codec_currentIndexChanged(int index);

private:
    Ui::IOSettingsForm *ui;

    SerialIOService    mSerialIOService;
    UdpIOService       mUdpIOService;
    TcpClientIOService mTcpClientIOService;
    TcpServerIOService mTcpServerIOService;
    QString         mIniFile;
    qint8    tcpServerConnectNum = 0;
    ConnectMode connectMode = None;
    QFile receiveFile;
    QTextStream receiveTextStream;
};

#endif // IOSETTINGSFORM_H
