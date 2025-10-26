#ifndef XMODEM_H
#define XMODEM_H

#include <QObject>
#include <QTimer>
#include <QFile>
#include <QDebug>

class Xmodem : public QObject
{
    Q_OBJECT
public:
    explicit Xmodem(QObject *parent = nullptr);

    enum XmodemState{
        IOConnectState,
        SendPercent,
        SendTransferState,
        SendInfo,
        SendWarning,
        SendError
    };
    Q_ENUM(XmodemState)

    enum SEND_STATE{
        IDLE = 0,
        WAIT_C,
        XMODEM_SEND,
        XMODEM_SEND_DOWN,
        XMODEM_SEND_ALL_FINISH,
        XMODEM_SEND_SEND_EOT,
        XMODEM_SEND_WAIT_EOT_ACK
    };
    Q_ENUM(SEND_STATE)

    void    StartSendXmodem(QString XmodemMode, QString FilePath);
    void    CancelSendXmodem( );
    quint16 crc16_ccitt(const quint8 *ptr, qint32 len);
    void    XmodemTransfer();

public slots:
    void onReadBytes(QByteArray bytes);
    void onXmodemStateChange(Xmodem::XmodemState type,QString state);
    void onMainTimeout();
signals:
    void sendBytes(QByteArray bytes);
    void xmodemStateChange(Xmodem::XmodemState type,QString state);

private:
    bool IOConnect = false;

    QTimer     MainTimer;
    QString    mXmodemMode;
    bool       startTransfer = false;
    QFile      file;
    QByteArray XmodeArray;
    qint32     MainTimerCount  = 0;
    qint32     XmodemSendCount = 0;
    uint32_t   packetNum       = 0;          // 第一包序号从 1 开始
    int        kPayload        = 0;       // 一帧中的数据实际有效数据量
    SEND_STATE send_state;
};

#endif // XMODEM_H
