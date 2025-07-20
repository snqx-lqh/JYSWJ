#include "sendareaform.h"
#include "ui_sendareaform.h"

SendAreaForm::SendAreaForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SendAreaForm)
{
    ui->setupUi(this);
}

SendAreaForm::~SendAreaForm()
{
    delete ui;
}

void SendAreaForm::onClearSend()
{
    ui->plainTextEdit_send->clear();
}

void SendAreaForm::on_btn_send_clicked()
{
    if (sendHexFlag) {
        // 1. 按十六进制发送
        QByteArray bytes = QByteArray::fromHex(
                               ui->plainTextEdit_send->toPlainText()
                               .toUtf8().simplified());   // 去掉空格、回车
        emit sendBytes(bytes);
    } else {
        // 2. 按普通文本发送
        QString text = ui->plainTextEdit_send->toPlainText();
        text.replace("\n", "\r\n");
        emit sendBytes(text.toLocal8Bit());
    }
}

void SendAreaForm::onChangeSendHexFlag(bool flag)
{
    sendHexFlag = flag;
}


void SendAreaForm::on_btn_selectFile_clicked()
{
    ui->le_File->setText(QFileDialog::getOpenFileName(
                             this,                       // 父窗口
                             tr("选择文件"),             // 标题
                             QDir::homePath(),           // 起始目录
                             tr("所有文件 (*);;文本文件 (*.txt)")  // 过滤器
                         ));
}


void SendAreaForm::on_btn_sendFile_clicked()
{
    emit sendFile(ui->le_File->text());
}

