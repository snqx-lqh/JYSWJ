#ifndef PROTOCOLTRANSFERFORM_H
#define PROTOCOLTRANSFERFORM_H

#include <QWidget>
#include <QTextCursor>
#include <QScrollBar>
#include <QApplication>
#include <QTimer>
#include <QDateTime>
#include <QFileDialog>
#include <QFileInfo>

namespace Ui {
class ProtocolTransferForm;
}

class ProtocolTransferForm : public QWidget
{
    Q_OBJECT

public:
    explicit ProtocolTransferForm(QWidget *parent = nullptr);
    ~ProtocolTransferForm();

    enum SEND_STATE{
        IDLE = 0,
        XMODEM_SEND,
        XMODEM_SEND_DOWN,
        XMODEM_SEND_ALL_FINISH,
        XMODEM_SEND_SEND_EOT,
        XMODEM_SEND_WAIT_EOT_ACK
    };
    Q_ENUM(SEND_STATE)

    void showMsg(QString color,QString msg);

    quint16 crc16_ccitt(const quint8 *ptr, qint32 len);
    void XmodemTransfer();

public slots:
    void onMainTimeout();
    void onReadBytes(QByteArray bytes);

private slots:
    void on_btn_CleanWindow_clicked();

    void on_btn_selectFile_clicked();

    void on_btn_StartSend_clicked();

    void on_btn_CancelSend_clicked();

signals:
    void sendBytes(QByteArray bytes);


private:
    Ui::ProtocolTransferForm *ui;

    QString lastDir = ".";

    QTimer MainTimer;
    bool   startTransfer = false;
    QFile file;
    QByteArray XmodeArray;
    qint32     MainTimerCount = 0;
    qint32     XmodemSendCount = 0;
    uint32_t   packetNum = 0;          // 第一包序号从 1 开始
    int        kPayload   = 0;       // 一帧中的数据实际有效数据量
    SEND_STATE send_state;
};

#endif // PROTOCOLTRANSFERFORM_H
