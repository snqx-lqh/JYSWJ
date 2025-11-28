#ifndef YMODEM_H
#define YMODEM_H

#include <QObject>
#include <QTimer>
#include <QFile>
#include <QDebug>
#include <QFileInfo>

#include "ringbuffer.h"

class Ymodem : public QObject
{
    Q_OBJECT
public:
    explicit Ymodem(QObject *parent = nullptr);
    enum YmodemState{
        IOConnectState,
        SendPercent,
        SendTransferState,
        SendInfo,
        SendWarning,
        SendError
    };
    Q_ENUM(YmodemState)

    enum SEND_STATE{
        IDLE = 0,
        WAIT_C,
        YMODEM_SEND_HEAD,
        YMODEM_SEND_HEAD_WAIT_CA,
        YMODEM_SEND_HEAD_WAIT_C,
        YMODEM_SEND,
        YMODEM_SEND_DOWN,
        YMODEM_SEND_ALL_FINISH,
        YMODEM_SEND_SEND_EOT,
        YMODEM_SEND_SEND_EOT_2,
        YMODEM_SEND_WAIT_EOT_ACK,
        YMODEM_SEND_WAIT_NEXT_FILE_C,
        YMODEM_SEND_END,
        YMODEM_SEND_END_WAIT_ACK,

    };
    Q_ENUM(SEND_STATE)

    void    StartSendYmodem(QString YmodemMode, QString FilePath);
    void    CancelSendYmodem( );
    quint16 crc16_ccitt(const quint8 *ptr, qint32 len);
    void    YmodemTransfer();
    void    YmodemTransferHead();
    void    YmodemTransferEnd();

public slots:
    void onReadBytes(QByteArray bytes);
    void onYmodemStateChange(Ymodem::YmodemState type,QString state);
    void onMainTimeout();
signals:
    void sendBytes(QByteArray bytes);
    void ymodemStateChange(Ymodem::YmodemState type,QString state);

private:
    bool IOConnect = false;

    RingBuffer<char> m_ringBuffer;
    QTimer     MainTimer;
    QString    mYmodemMode;
    bool       startTransfer = false;
    QFile      file;
    QByteArray YmodeArray;
    qint32     MainTimerCount  = 0;
    qint32     YmodemSendCount = 0;
    uint32_t   packetNum       = 0;          // 第一包序号从 1 开始
    int        kPayload        = 0;       // 一帧中的数据实际有效数据量
    SEND_STATE send_state;
};

#endif // YMODEM_H
