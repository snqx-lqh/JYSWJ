#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSettings>
#include <QDir>
#include <QDebug>
#include <QLabel>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QFileDialog>
#include <QDateTime>
#include <QAbstractNativeEventFilter>
#include <Windows.h>
#include <dbt.h>
#include <QList>
#include <QMessageBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>

#include "serialioservice.h"
#include "udpioservice.h"
#include "tcpserverioservice.h"
#include "tcpclientioservice.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow,QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    struct Row {
        QCheckBox   *check[2];
        QLineEdit   *edit[2];
        QPushButton *btn[2];
    };

    enum ConnectMode{
        Serial       = 0,
        Udp          = 1,
        TcpServer    = 2,
        TcpClient    = 3,
        None         = -1
    };
    Q_ENUM(ConnectMode)

    void loadSettings(QSettings*);
    void saveSettings(QSettings*);
    QString visualHex(const QByteArray &ba);
    void onAppendSendBytes(QByteArray Bytes);
    void multiSendInit();
    void sendBytes(QByteArray);
    void sendText(QCheckBox *checkHexBox,QString text);

    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);

public slots:

    void onReadBytes(QByteArray);
    void onSendCountChanged(uint32_t count);
    void onRecvCountChanged(uint32_t count);
protected:
    void closeEvent(QCloseEvent *event);

private slots:

    void on_btn_OpenSerial_clicked();

    void on_cmb_PortName_currentTextChanged(const QString &arg1);

    void on_cmb_BaudRate_currentTextChanged(const QString &arg1);

    void on_btn_Send_clicked();

    void on_btn_SelectFile_clicked();

    void on_btn_SendFile_clicked();

    void on_cb_RecvToFile_clicked(bool checked);

    void on_radioButton_BasicSend_clicked();

    void on_radioButton_MultiSend_clicked();

    void on_rB_BasicComunication_clicked();

    void on_rB_WaveShow_clicked();

    void on_cmb_StopBits_currentTextChanged(const QString &arg1);

    void on_cmb_DataBits_currentTextChanged(const QString &arg1);

    void on_cmb_Parity_currentTextChanged(const QString &arg1);

    void on_btn_SendClear_clicked();

    void on_btn_RecvClear_clicked();

    void on_action_version_triggered();

    void on_btn_UdpConnect_clicked();

    void on_btn_Listen_clicked();

    void on_btn_TcpClientConnect_clicked();

private:
    Ui::MainWindow *ui;

    QString   iniFile;
    QSettings *settings;
    QString ConnectInfo;
    QLabel *lb_ConnectInfo;
    QLabel *lb_SendRecvInfo;
    SerialIOService *serialIOService;
    QFile         receiveFile;
    QTextStream   receiveTextStream;
    QVector<Row> rows;
    QUdpSocket   *udpSocket;
    UdpIOService *udpIOService;
    TcpServerIOService *tcpServerIOService;
    QTcpSocket *tcpSocket;
    TcpClientIOService *tcpClientIOService;

    MainWindow::ConnectMode connectMode = None;
    uint32_t sendCount=0;
    uint32_t recvCount=0;
    qint8    tcpServerConnectNum = 0;
};
#endif // MAINWINDOW_H
