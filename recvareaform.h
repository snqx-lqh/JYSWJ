#ifndef RECVAREAFORM_H
#define RECVAREAFORM_H

#include <QWidget>
#include "terminal.h"
#include "common.h"
#include "waveshowform.h"
#include <QThread>


namespace Ui {
class RecvAreaForm;
}

class RecvAreaForm : public QWidget
{
    Q_OBJECT

public:
    explicit RecvAreaForm(QWidget *parent = nullptr);
    ~RecvAreaForm();

    void stateInit();
    void loadSettings();
    void saveSettings();


    void onStateChange(STATE_CHANGE_TYPE_T type,int state);


public slots:
    void onReadBytes(QByteArray bytes);
    void onAppendSendData(QByteArray bytes);
signals:
    void sendBytes(QByteArray bytes);
    void stateChange(STATE_CHANGE_TYPE_T type,int state);
private slots:
    void on_checkBox_ShowHex_stateChanged(int arg1);

    void on_checkBox_ShowDate_stateChanged(int arg1);

    void on_comboBox_codec_currentTextChanged(const QString &arg1);

    void on_checkBox_SendShow_stateChanged(int arg1);

    void on_pushButton_ClearRecv_clicked();

    void on_checkBox_RecvFile_stateChanged(int arg1);

    void on_checkBox_RecvFile_clicked(bool checked);

private:
    Ui::RecvAreaForm *ui;
    WaveShowForm mWaveShowForm;
    QThread mThread;
    QString         mIniFile;
    QFile         receiveFile;
    QTextStream   receiveTextStream;
};

#endif // RECVAREAFORM_H
