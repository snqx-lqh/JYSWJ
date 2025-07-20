#ifndef SENDAREAFORM_H
#define SENDAREAFORM_H

#include <QWidget>
#include <QFile>
#include <QDir>
#include <QFileDialog>

namespace Ui {
class SendAreaForm;
}

class SendAreaForm : public QWidget
{
    Q_OBJECT

public:
    explicit SendAreaForm(QWidget *parent = nullptr);
    ~SendAreaForm();

signals:
    void sendString(QString);
    void sendBytes(QByteArray);
    void sendFile(QString);

public slots:
    void onClearSend();
    void onChangeSendHexFlag(bool);

private slots:
    void on_btn_send_clicked();

    void on_btn_selectFile_clicked();

    void on_btn_sendFile_clicked();

private:
    Ui::SendAreaForm *ui;
    bool sendHexFlag  = false;
};

#endif // SENDAREAFORM_H
