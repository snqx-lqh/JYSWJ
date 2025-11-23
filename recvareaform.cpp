#include "recvareaform.h"
#include "ui_recvareaform.h"

RecvAreaForm::RecvAreaForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RecvAreaForm)
{
    ui->setupUi(this);

    QDir dir(QCoreApplication::applicationDirPath());
    mIniFile = dir.filePath("Config/settings.ini");

    ui->tabWidget->addTab(&mWaveShowForm,"波形显示");
    connect(ui->plainTextEdit,&Terminal::sendBytes,this,&RecvAreaForm::sendBytes);

    ui->comboBox_codec->addItem("GBK");
    ui->comboBox_codec->addItem("UTF-8");

    loadSettings();

    ui->comboBox_codec->setCurrentText(TextCodeC);
}

RecvAreaForm::~RecvAreaForm()
{
    saveSettings();
    delete ui;
}

void RecvAreaForm::stateInit()
{
    if(ui->checkBox_SendShow->isChecked()){
        emit stateChange(RecvShowSend_State,1);
    }else{
        emit stateChange(RecvShowSend_State,0);
    }

    if(ui->comboBox_codec->currentText() == "GBK"){
        emit stateChange(TextCodec, 0);
    }else if(ui->comboBox_codec->currentText() == "UTF-8"){
        emit stateChange(TextCodec, 1);
    }
}

void RecvAreaForm::loadSettings()
{
    QSettings settings(mIniFile,QSettings::IniFormat);
    settings.beginGroup("RecvAreaForm");

    ui->checkBox_ShowHex->setChecked(settings.value("HexShow","false").toBool());
    ui->checkBox_ShowDate->setChecked(settings.value("DateShow","false").toBool());
    ui->checkBox_SendShow->setChecked(settings.value("SendShow","false").toBool());
    TextCodeC = settings.value("comboBox_codec","GBK").toString();

    settings.endGroup();
}

void RecvAreaForm::saveSettings()
{
    QSettings settings(mIniFile,QSettings::IniFormat);
    settings.beginGroup("RecvAreaForm");

    settings.setValue("HexShow",ui->checkBox_ShowHex->isChecked());
    settings.setValue("DateShow",ui->checkBox_ShowDate->isChecked());
    settings.setValue("SendShow",ui->checkBox_SendShow->isChecked());
    settings.setValue("comboBox_codec",ui->comboBox_codec->currentText());

    settings.endGroup();
}

void RecvAreaForm::onStateChange(STATE_CHANGE_TYPE_T type, int state)
{

}

void RecvAreaForm::onReadBytes(QByteArray bytes)
{
    ui->plainTextEdit->onReadBytes(bytes);
    if(receiveFile.isOpen()){
        //receiveTextStream<<bytes;
        receiveFile.write(bytes);
        receiveFile.flush();
    }
    mWaveShowForm.onReadBytes(bytes);
}

void RecvAreaForm::onAppendSendData(QByteArray bytes)
{
    ui->plainTextEdit->appendSendData(bytes);
}

void RecvAreaForm::on_checkBox_ShowHex_stateChanged(int arg1)
{
    if(arg1 == 0){
        ui->plainTextEdit->setShowHexState(false);
    }else{
        ui->plainTextEdit->setShowHexState(true);
    }
}


void RecvAreaForm::on_checkBox_ShowDate_stateChanged(int arg1)
{
    if(arg1 == 0){
        ui->plainTextEdit->setShowDateState(false);
    }else{
        ui->plainTextEdit->setShowDateState(true);
    }
}

void RecvAreaForm::on_comboBox_codec_currentTextChanged(const QString &arg1)
{
    if(arg1 == "GBK"){
        ui->plainTextEdit->setEncoding("GBK");
        emit stateChange(TextCodec, 0);
    }else if(arg1 == "UTF-8"){
        ui->plainTextEdit->setEncoding("UTF-8");
        emit stateChange(TextCodec, 1);
    }
}

void RecvAreaForm::on_checkBox_SendShow_stateChanged(int arg1)
{
    emit stateChange(RecvShowSend_State,arg1);
}

void RecvAreaForm::on_pushButton_ClearRecv_clicked()
{
    ui->plainTextEdit->clear();
    mWaveShowForm.clearRecv();
}


void RecvAreaForm::on_checkBox_RecvFile_stateChanged(int arg1)
{



}


void RecvAreaForm::on_checkBox_RecvFile_clicked(bool checked)
{
    if(checked == true){
        QString filePath = QDir::currentPath() + "/receiveData_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".txt";

        receiveFile.setFileName(filePath);
        if(receiveFile.open(QIODevice::WriteOnly | QIODevice::Text)){
            receiveTextStream.setDevice(&receiveFile);
        }

        QMessageBox msg;
        msg.setText("文件将保存在"+filePath);
        msg.show();
        msg.exec();
    }else{
        if(receiveFile.isOpen()){
            receiveFile.close();
        }
    }
}

