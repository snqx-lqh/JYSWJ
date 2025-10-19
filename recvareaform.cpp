#include "recvareaform.h"
#include "ui_recvareaform.h"

RecvAreaForm::RecvAreaForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RecvAreaForm)
{
    ui->setupUi(this);

    ui->tabWidget->addTab(&mWaveShowForm,"波形显示");
    connect(ui->plainTextEdit,&Terminal::sendBytes,this,&RecvAreaForm::sendBytes);

}

RecvAreaForm::~RecvAreaForm()
{
    delete ui;
}

void RecvAreaForm::onStateChange(STATE_CHANGE_TYPE_T type, int state)
{
    if(type == RecvShowHex_State){
        if(state == 0){
            ui->plainTextEdit->setShowHexState(false);
        }else{
            ui->plainTextEdit->setShowHexState(true);
        }
    }else if(type == RecvShowDate_State){
        if(state == 0){
            ui->plainTextEdit->setShowDateState(false);
        }else{
            ui->plainTextEdit->setShowDateState(true);
        }
    }else if(type == RecvClear_State){
        ui->plainTextEdit->clear();
        mWaveShowForm.clearRecv();
    }else if(type == TextCodec){
        if(state == 0){
            ui->plainTextEdit->setEncoding("GBK");
        }else if(state == 1){
            ui->plainTextEdit->setEncoding("UTF-8");
        }
    }
}

void RecvAreaForm::onReadBytes(QByteArray bytes)
{
    ui->plainTextEdit->onReadBytes(bytes);
    mWaveShowForm.onReadBytes(bytes);
}

void RecvAreaForm::onAppendSendData(QByteArray bytes)
{
    ui->plainTextEdit->appendSendData(bytes);
}
