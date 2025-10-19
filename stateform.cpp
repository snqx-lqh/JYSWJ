#include "stateform.h"
#include "ui_stateform.h"

StateForm::StateForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StateForm)
{
    ui->setupUi(this);

    // 发送接收字符计数
    ui->label_SendRecvCount->setText(QString("发送：%1 接收：%2").arg(0, 8).arg(0, 8));
    // 状态栏增加串口信息显示
    ui->label_ConnectInfo->setText(tr("COM,110,8,1,None,None"));
}

StateForm::~StateForm()
{
    delete ui;
}

void StateForm::onStateChange(STATE_CHANGE_TYPE_T type, int state)
{
    if(type == RecvCount){
        recvCount += state;
        ui->label_SendRecvCount->setText(QString("发送：%1 接收：%2").arg(sendCount, 8).arg(recvCount, 8));
    }else if(type == SendCount){
        sendCount += state;
        ui->label_SendRecvCount->setText(QString("发送：%1 接收：%2").arg(sendCount, 8).arg(recvCount, 8));
    }else if(type == RecvClear_State){
        recvCount = 0;
        sendCount = 0;
        ui->label_SendRecvCount->setText(QString("发送：%1 接收：%2").arg(sendCount, 8).arg(recvCount, 8));
    }
}

void StateForm::onConnectInfo(QString Info)
{
    ui->label_ConnectInfo->setText(Info);
}
