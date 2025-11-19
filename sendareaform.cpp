#include "sendareaform.h"
#include "ui_sendareaform.h"

SendAreaForm::SendAreaForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SendAreaForm)
{
    ui->setupUi(this);

    QDir dir(QCoreApplication::applicationDirPath());
    mIniFile = dir.filePath("Config/settings.ini");

    ui->tabWidget->addTab(&mProtocolTransferForm,"协议发送");
    ui->tabWidget->addTab(&mFileConvertForm,"文件转换");

    connect(&mProtocolTransferForm,&ProtocolTransferForm::sendBytes,this,&SendAreaForm::sendBytes);
    connect(this,&SendAreaForm::protocolStateChange,&mProtocolTransferForm,&ProtocolTransferForm::onProtocolStateChange);

    SendCycleTimer = new QTimer;
    connect(SendCycleTimer,&QTimer::timeout,this,[this](){
        on_pushButton_Send_clicked();
    });
    ui->comboBox_HistoryFile->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    loadSettings();
}

SendAreaForm::~SendAreaForm()
{
    saveSettings();
    delete ui;
}

void SendAreaForm::loadSettings()
{
    QSettings settings(mIniFile,QSettings::IniFormat);
    settings.beginGroup("SendAreaForm");

    auto comboBox_HistoryFile_count              = settings.value("comboBox_HistoryFile_count","0");
    auto comboBox_HistoryFile_currentText        = settings.value("comboBox_HistoryFile_currentText","");
    for(int i = 0;i < comboBox_HistoryFile_count.toInt();i++){
        QString comboBox_HistoryFile = settings.value(QString("comboBox_HistoryFile_%1").arg(i),"").toString();
        bool exists = QFileInfo::exists(comboBox_HistoryFile);
        if(exists)
            ui->comboBox_HistoryFile->addItem(comboBox_HistoryFile);
    }
    ui->lineEdit_SendCycle->setText(settings.value(QString("lineEdit_SendCycle"),"1000").toString());
    ui->checkBox_HexSend->setChecked(settings.value(QString("checkBox_HexSend"),ui->checkBox_HexSend->isChecked()).toBool());
    ui->plainTextEdit->appendPlainText(settings.value(QString("plainTextEdit"),"").toString());
    settings.endGroup();

}

void SendAreaForm::saveSettings()
{
    QSettings settings(mIniFile,QSettings::IniFormat);
    settings.beginGroup("SendAreaForm");

    settings.setValue("comboBox_HistoryFile_count", ui->comboBox_HistoryFile->count());
    settings.setValue("comboBox_HistoryFile_currentText", ui->comboBox_HistoryFile->currentText());
    for(int i = 0;i<ui->comboBox_HistoryFile->count();i++){
        settings.setValue(QString("comboBox_HistoryFile_%1").arg(i), ui->comboBox_HistoryFile->itemText(i));
    }
    settings.setValue("lineEdit_SendCycle", ui->lineEdit_SendCycle->text());
    settings.setValue("checkBox_HexSend", ui->checkBox_HexSend->isChecked());
    settings.setValue("plainTextEdit", ui->plainTextEdit->toPlainText());

    settings.endGroup();
}

QProgressBar *SendAreaForm::getProgressBar()
{
    return ui->progressBar;
}

void SendAreaForm::onStateChange(STATE_CHANGE_TYPE_T type, int state)
{
    if(type == IOConnect_State){
        if(state == 0){
            connectState = false;
        }
        else {
            connectState = true;
        }
        emit protocolStateChange(IOConnect_State,state);
    }else if(type == IOSendNewLine_State){
        if(state == 0) sendNewLineState = false;
        else sendNewLineState = true;
    }else if(type == TextCodec){
        if(state == 0) TextCodeC = "GBK";
        else if(state == 1) TextCodeC = "UTF-8";
    }
}

void SendAreaForm::onReadBytes(QByteArray bytes)
{
    mProtocolTransferForm.onReadBytes(bytes);
}


void SendAreaForm::on_pushButton_Clear_clicked()
{
    ui->plainTextEdit->clear();
}


void SendAreaForm::on_pushButton_SelectFile_clicked()
{
    QString path;
    path = QFileInfo(ui->comboBox_HistoryFile->currentText()).filePath();
    QString filePath = QFileDialog::getOpenFileName(
                this,                               // 父窗口
                tr("选择文件"),                      // 标题
                path,                               // 起始目录
                tr("所有文件 (*);;文本文件 (*.txt)")  // 过滤器
            );
    if(filePath.isEmpty()) return;
    ui->comboBox_HistoryFile->addItem(filePath);
    ui->comboBox_HistoryFile->setCurrentText(filePath);
    if(ui->comboBox_HistoryFile->count() > 5){
        ui->comboBox_HistoryFile->removeItem(0);
    }
}


void SendAreaForm::on_pushButton_Send_clicked()
{
    if(connectState == false){
        QMessageBox::warning(nullptr,"警告","通信IO未连接");
        return;
    }
    QString text = ui->plainTextEdit->toPlainText();
    //text.replace("\\r","\r").replace("\\n","\n");
    text.replace("\n","\r\n");
    if (ui->checkBox_HexSend->isChecked()) {
        // 1. 按十六进制发送
        QByteArray bytes = QByteArray::fromHex(
                               text.toUtf8().simplified());   // 去掉空格、回车
        if(sendNewLineState) {
            bytes.append(0x0D);
            bytes.append(0x0A);
        }
        emit sendBytes(bytes);
    } else {
        QByteArray bytes;
        if (TextCodeC == "GBK") {
            // GB18030 向下兼容 GBK， Qt5/6 都认
            static QTextCodec *gbk = QTextCodec::codecForName("GB18030");
            if (gbk) bytes = gbk->fromUnicode(text);
            else     bytes = text.toLocal8Bit();   // 兜底
        } else if (TextCodeC == "UTF-8"){
            bytes = text.toUtf8();                 // UTF-8
        }
        if(sendNewLineState) {
            bytes.append(0x0D);
            bytes.append(0x0A);
        }
        emit sendBytes(bytes);
    }
}


void SendAreaForm::on_pushButton_SendFile_clicked()
{
    emit sendFile(ui->comboBox_HistoryFile->currentText());
}

void SendAreaForm::on_checkBox_SendCycle_stateChanged(int arg1)
{
    if(2 == arg1){
        if(connectState == false){
            QMessageBox::warning(nullptr,"警告","通信IO未连接");
            SendCycleTimer->stop();
            ui->checkBox_SendCycle->setChecked(false);
            return;
        }
        SendCycleTimer->start(ui->lineEdit_SendCycle->text().toUInt());
    }else{
        // 未勾选
        SendCycleTimer->stop();
    }
}

