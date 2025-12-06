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

    enum YmodemCMD{
        SOH = 0X01,
        STX = 0X02,
        EOT = 0X04,
        ACK = 0X06,
        NAK = 0X15,
        CAN = 0X18
    };
    Q_ENUM(YmodemCMD)

    enum SEND_STATE{
        IDLE = 0,
        WAIT_C,                        ///< 等待C字符
        YMODEM_SEND_HEAD,              ///< 发送头 包含文件名 文件大小
        YMODEM_SEND_HEAD_WAIT_CA,      ///< 发送完头后的 ACK 和 C

        YMODEM_SEND,                   ///< 发送一帧字符
        YMODEM_SEND_RETRY,             ///< 重发送上帧字符
        YMODEM_SEND_DOWN,              ///< 一帧发送完成

        YMODEM_SEND_SEND_EOT,          ///< 发送结尾
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
    void    YmodemTransfer(bool retry);
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

    int retryCount = 0;
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
