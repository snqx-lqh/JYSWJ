#ifndef MULTISENDFORM_H
#define MULTISENDFORM_H

#include <QWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QQueue>
#include <QSettings>
#include <QDir>
#include <QInputDialog>
#include <QDebug>
#include <QMessageBox>
#include <QTimer>
#include <QTextCodec>
#include "common.h"

namespace Ui {
class MultiSendForm;
}

class MultiSendForm : public QWidget
{
    Q_OBJECT

public:
    explicit MultiSendForm(QWidget *parent = nullptr);
    ~MultiSendForm();

    struct MultiSendItem
    {
        int          itemIndex;
        QCheckBox   *check;
        QLineEdit   *edit;
        QPushButton *btn;
        QLineEdit   *indexEdit;
        QLineEdit   *timerEdit;
    };

    void UiInit(void);
    void loadSettings();
    void saveSettings();

    void sendText(MultiSendItem item);
public slots:
    void onMultiSendCycleTimerOut();
    void onStateChange(STATE_CHANGE_TYPE_T type,int state);

signals:
    void sendBytes(QByteArray bytes);

private slots:
    void on_pushButton_NewIni_clicked();

    void on_comboBox_IniFile_currentTextChanged(const QString &arg1);

    void on_checkBox_CycleSend_stateChanged(int arg1);

private:
    Ui::MultiSendForm *ui;

    QVector<MultiSendItem> MultiSendItems;
    QVector<MultiSendItem> MultiSendItemsTemp;
    QQueue<MultiSendItem>  txQueue;
    int txQueueGetIndex = 0;
    QStringList MultiSendInfo;
    int    mulSendItemMaxNum = 50;
    QString         mIniFile;
    bool   initFinished = false;
    bool   connectState = false;
    QTimer *MultiSendCycleTimer;
    bool   sendNewLineState = false;
    QString TextCodeC;
};


#endif // MULTISENDFORM_H
