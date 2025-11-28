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
#include "ymodem.h"

namespace Ui {
class ProtocolTransferForm;
}

class ProtocolTransferForm : public QWidget
{
    Q_OBJECT

public:
    explicit ProtocolTransferForm(QWidget *parent = nullptr);
    ~ProtocolTransferForm();

    void showMsg(QString color,QString msg);

    void loadSettings();
    void saveSettings();

public slots:
    void onReadBytes(QByteArray bytes);
    void onProtocolStateChange(STATE_CHANGE_TYPE_T type,int state);
    void onXmodemStateChange(Xmodem::XmodemState type,QString state);
    void onYmodemStateChange(Ymodem::YmodemState type,QString state);

private slots:
    void on_btn_CleanWindow_clicked();

    void on_btn_selectFile_clicked();

    void on_btn_StartSend_clicked();

    void on_btn_CancelSend_clicked();

signals:
    void sendBytes(QByteArray bytes);
    void xmodemStateChange(Xmodem::XmodemState type,QString state);
    void ymodemStateChange(Ymodem::YmodemState type,QString state);

private:
    Ui::ProtocolTransferForm *ui;

    QString lastDir = ".";
    QString    m_iniFile;
    Xmodem     mXmodem;
    Ymodem     mYmodem;

    QString ProtocolMethod = ""; ///< 协议名
    QString SendFilePath   = ""; ///< 发送文件路径
};

#endif // PROTOCOLTRANSFERFORM_H
