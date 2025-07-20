#ifndef RECVAREAFORM_H
#define RECVAREAFORM_H

#include <QWidget>
#include <QDebug>
#include <QTime>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

namespace Ui {
class RecvAreaForm;
}

class RecvAreaForm : public QWidget
{
    Q_OBJECT

public:
    explicit RecvAreaForm(QWidget *parent = nullptr);
    ~RecvAreaForm();

    QString visualHex(const QByteArray &ba);

public slots:
    void onReadBytes(QByteArray);
    void onAppendSendBytes(QByteArray  );
    void onClearRecv();
    void onChangeAddTimeFlag(bool);
    void onChangeHexShowFlag(bool);
    void onChangeSendShowFlag(bool);
    void onChangeSendHexFlag(bool);
    void onSaveRecvEditData();

private:
    Ui::RecvAreaForm *ui;

    bool addTimeFlag = false;
    bool hexShowFlag = false;
    bool sendShowFlag = false;
    bool sendHexFlag  = false;
};

#endif // RECVAREAFORM_H
