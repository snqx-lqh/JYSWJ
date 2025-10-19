#ifndef SENDAREAFORM_H
#define SENDAREAFORM_H

#include <QWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QFile>
#include <QSettings>
#include <QProgressBar>
#include <QMessageBox>

#include "common.h"
#include "protocoltransferform.h"
#include "fileconvertform.h"

namespace Ui {
class SendAreaForm;
}

class SendAreaForm : public QWidget
{
    Q_OBJECT

public:
    explicit SendAreaForm(QWidget *parent = nullptr);
    ~SendAreaForm();
    void loadSettings();
    void saveSettings();
    QProgressBar *getProgressBar();
public slots:
    void onStateChange(STATE_CHANGE_TYPE_T type,int state);
    void onReadBytes(QByteArray bytes);

private slots:
    void on_pushButton_Clear_clicked();

    void on_pushButton_SelectFile_clicked();

    void on_pushButton_Send_clicked();

    void on_pushButton_SendFile_clicked();


    void on_checkBox_SendCycle_stateChanged(int arg1);

signals:
    void sendBytes(QByteArray bytes);
    void sendFile(QString file);

private:
    Ui::SendAreaForm *ui;
    QString         mIniFile;
    ProtocolTransferForm mProtocolTransferForm;
    FileConvertForm mFileConvertForm;
    bool   sendNewLineState = false;
    bool   connectState     = false;
    QTimer *SendCycleTimer;
    QString TextCodeC;
};

#endif // SENDAREAFORM_H
