#include "toolbarform.h"
#include "ui_toolbarform.h"

ToolBarForm::ToolBarForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ToolBarForm)
{
    ui->setupUi(this);

    iconExpand.addFile(":/images/images/expand.png");
    iconConnect.addFile(":/images/images/connect.png");
    iconDisconnect.addFile(":/images/images/disconnect.png");
    iconMultiSend.addFile(":/images/images/multisend.png");

    ui->pushButton_Connect->setMinimumSize(32,32);
    ui->pushButton_Connect->setMaximumSize(32,32);
    ui->pushButton_Connect->setIcon(iconDisconnect);
    ui->pushButton_Connect->setIconSize(QSize(32,32));
    ui->pushButton_Connect->setCursor(Qt::PointingHandCursor);

    ui->pushButton_ZK->setMinimumSize(32,32);
    ui->pushButton_ZK->setMaximumSize(32,32);
    ui->pushButton_ZK->setIcon(iconExpand);
    ui->pushButton_ZK->setIconSize(QSize(32,32));
    ui->pushButton_ZK->setToolTip("展开或关闭IO设置");
    ui->pushButton_ZK->setCursor(Qt::PointingHandCursor);

    ui->pushButton_MultiSend->setMinimumSize(32,32);
    ui->pushButton_MultiSend->setMaximumSize(32,32);
    ui->pushButton_MultiSend->setIcon(iconMultiSend);
    ui->pushButton_MultiSend->setIconSize(QSize(32,32));
    ui->pushButton_MultiSend->setToolTip("多字符发送");
    ui->pushButton_MultiSend->setCursor(Qt::PointingHandCursor);




}

ToolBarForm::~ToolBarForm()
{
    delete ui;
}

void ToolBarForm::onStateChange(STATE_CHANGE_TYPE_T type,int state)
{
    if(type == IOConnect_State){
        if(state != 0){
            ui->pushButton_Connect->setIcon(iconConnect);
            connectState = true;
        }else{
            ui->pushButton_Connect->setIcon(iconDisconnect);
            connectState = false;
        }
    }
}

void ToolBarForm::on_pushButton_Connect_clicked()
{
    emit stateChange(IOConnect_State,connectState);
}


void ToolBarForm::on_pushButton_ZK_clicked()
{
    expandState = !expandState;
    emit stateChange(IOSETTING_VISIBLE,expandState);
}


void ToolBarForm::on_pushButton_MultiSend_clicked()
{
    multiSendState = !multiSendState;
    emit stateChange(MULTISEND_VISIBLE,multiSendState);
}

