#include "recvareaform.h"
#include "ui_recvareaform.h"

RecvAreaForm::RecvAreaForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RecvAreaForm)
{
    ui->setupUi(this);
}

RecvAreaForm::~RecvAreaForm()
{
    delete ui;
}

// 暂时未使用,功能不全
QString RecvAreaForm::visualHex(const QByteArray &ba)
{
    QString out;
    out.reserve(ba.size() * 4);          // 预分配
    for (QChar c : ba) {
//        if (c < 0x20 || c == 0x7F)       // 控制字符
//            out += QChar(0x25A1);
//        else
            out += c;
    }
    return out;
}

void RecvAreaForm::onReadBytes(QByteArray Bytes)
{
    QString chunk;
    if (addTimeFlag) {
        chunk = '\n' + QDateTime::currentDateTime()
                       .toString("[yy-MM-dd hh:mm:ss.zzz]收<-");
    }
    chunk += hexShowFlag ? Bytes.toHex(' ') : visualHex(Bytes);
    ui->textBrowser_recv->insertPlainText(chunk);
    ui->textBrowser_recv->moveCursor(QTextCursor::End);
}

void RecvAreaForm::onAppendSendBytes(QByteArray Bytes)
{
    if(sendShowFlag == true){
        QString chunk;
        if (addTimeFlag) {
            chunk = '\n' + QDateTime::currentDateTime()
                           .toString("[yy-MM-dd hh:mm:ss.zzz]发->");
        }
        chunk += hexShowFlag ? Bytes.toHex(' ') : visualHex(Bytes);
        ui->textBrowser_recv->insertPlainText(chunk);
        ui->textBrowser_recv->moveCursor(QTextCursor::End);
    }
}



void RecvAreaForm::onClearRecv()
{
    ui->textBrowser_recv->clear();
}

void RecvAreaForm::onChangeAddTimeFlag(bool flag)
{
    addTimeFlag = flag;
}

void RecvAreaForm::onChangeHexShowFlag(bool flag)
{
    hexShowFlag = flag;
}

void RecvAreaForm::onChangeSendShowFlag(bool flag)
{
    sendShowFlag = flag;
}

void RecvAreaForm::onChangeSendHexFlag(bool flag)
{
    sendHexFlag = flag;
}

void RecvAreaForm::onSaveRecvEditData()
{
    QString filePath = QDir::currentPath() + "/saveReceiveFile_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".txt";

    QFile saveReceiveFile(filePath);
    QTextStream saveReceiveTextStream;
    if(saveReceiveFile.open(QIODevice::WriteOnly | QIODevice::Text)){
        saveReceiveTextStream.setDevice(&saveReceiveFile);
        saveReceiveTextStream<<ui->textBrowser_recv->toPlainText();
        saveReceiveFile.close();
    }

    QMessageBox msg;
    msg.setText("接收框处理文件保存在"+filePath);
    msg.show();
    msg.exec();
}
