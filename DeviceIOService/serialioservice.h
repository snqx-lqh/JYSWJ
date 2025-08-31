#ifndef SERIALIOSERVICE_H
#define SERIALIOSERVICE_H

#include <QObject>
#include <QComboBox>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QFile>
#include <QDebug>
#include <QMessageBox>
#include <QDir>
#include <QCoreApplication>
#include <QSettings>
#include <QProgressBar>

#include <QTimer>

class SerialIOService : public QObject
{
    Q_OBJECT
public:
    explicit SerialIOService    (QObject *parent = nullptr);
    ~SerialIOService();

    bool openSerial             (void);
    void closeSerial            (void);
    bool isSerialOpen           (void);

    void sendBytes              (QByteArray bytes);
    void sendFile               (QString    filePath);
    void writeNextChunk         ();
    void handleBytesWritten     (qint64 bytes);

    void scanAvailableSerialPort(QComboBox* cmb);
    void addBaudItems           (QComboBox* cmb);
    void addCommonBaudItem      (QComboBox* cmb);
    void addCustomBaudItem      (QComboBox* cmb, QString serialBaud, bool sortFlag = false);
    void delCustomBaudItem      (QComboBox* cmb, int     index     , bool sortFlag = false);
    void updateBaudItem         (QComboBox* cmb);

    void loadSettings           (void);
    void saveSettings           (void);

    void setSerialPortName      (QString portName);
    void setSerialBaudRate      (QString baud);
    void setSerialStopBits      (QString dataBits);
    void setSerialDataBits      (QString stopBits);
    void setSerialParity        (QString parity);
    void setSerialFlowControl   (QString flowControl);
    QString getSerialPortName   (void);
    QString getSerialBaudRate   (void);
    QString getSerialStopBits   (void);
    QString getSerialDataBits   (void);
    QString getSerialParity     (void);
    QString getSerialFlowControl(void);
    QString getSerialConnectInfo(void);

    void setProgressBar(QProgressBar *_progressBar);

public slots:
    void onReadReady();

signals:
    void readBytes              (QByteArray bytes);
    void sendBytesCount         (uint32_t   count);
    void recvBytesCount         (uint32_t   count);

private:
    QSerialPort  *serialPort=nullptr;
    QStringList   serialBaudList;
    QProgressBar *progressBar=nullptr;

    // 类成员变量
    QFile   file;
    qint64  fileLength;
    qint64  fileReadLength;
};

#endif // SERIALIOSERVICE_H
