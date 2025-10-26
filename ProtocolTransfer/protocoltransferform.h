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
#include <QSettings>
#include <QDir>
#include <QDebug>
#include "common.h"
#include "xmodem.h"

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

    void loadSettings();
    void saveSettings();

public slots:
    void onReadBytes(QByteArray bytes);
    void onProtocolStateChange(STATE_CHANGE_TYPE_T type,int state);
    void onXmodemStateChange(Xmodem::XmodemState type,QString state);


private slots:
    void on_btn_CleanWindow_clicked();

    void on_btn_selectFile_clicked();

    void on_btn_StartSend_clicked();

    void on_btn_CancelSend_clicked();

signals:
    void sendBytes(QByteArray bytes);
    void xmodemStateChange(Xmodem::XmodemState type,QString state);

private:
    Ui::ProtocolTransferForm *ui;

    QString lastDir = ".";
    QString    m_iniFile;
    Xmodem     mXmodem;

    QString ProtocolMethod = ""; ///< 协议名
    QString SendFilePath   = ""; ///< 发送文件路径
};

#endif // PROTOCOLTRANSFERFORM_H
