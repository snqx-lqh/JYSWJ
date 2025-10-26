#include "protocoltransferform.h"
#include "ui_protocoltransferform.h"

ProtocolTransferForm::ProtocolTransferForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProtocolTransferForm)
{
    ui->setupUi(this);

    QDir dir(QCoreApplication::applicationDirPath());
    m_iniFile = dir.filePath("Config/settings.ini");

    ui->comboBox_HistoryFile->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    connect(&mXmodem,&Xmodem::sendBytes,this,&ProtocolTransferForm::sendBytes);
    connect(&mXmodem,&Xmodem::xmodemStateChange,this,&ProtocolTransferForm::onXmodemStateChange);
    connect(this,&ProtocolTransferForm::xmodemStateChange,&mXmodem,&Xmodem::onXmodemStateChange);

    loadSettings();
}

ProtocolTransferForm::~ProtocolTransferForm()
{
    saveSettings();
    delete ui;
}

void ProtocolTransferForm::showMsg(QString color, QString msg)
{
    QString timeStr = QDateTime::currentDateTime().toString("[yy-MM-dd hh:mm:ss]");
    msg = timeStr + msg;

    QString html = QStringLiteral("<font color=\"%1\">%2</font>")
                   .arg(color.toHtmlEscaped(), msg.toHtmlEscaped());
    ui->plainTextEdit->appendHtml(html);
}

void ProtocolTransferForm::loadSettings()
{
    QSettings settings(m_iniFile,QSettings::IniFormat);
    settings.beginGroup("ProtocolTransferForm");

    int History_Count = settings.value("History_Count",0).toInt();

    for(int i=0;i<History_Count;i++){
        QString temp = settings.value(QString("History_File%1").arg(i)).toString();
        bool exists = QFileInfo::exists(temp);
        if(exists)
            ui->comboBox_HistoryFile->addItem(temp);
    }
    ui->cmb_SendMode->setCurrentText(settings.value("cmb_SendMode",ui->cmb_SendMode->currentText()).toString());
    ui->comboBox_HistoryFile->setCurrentText(settings.value("History_File",ui->comboBox_HistoryFile->currentText()).toString());
    settings.endGroup();
}

void ProtocolTransferForm::saveSettings()
{
    QSettings settings(m_iniFile,QSettings::IniFormat);
    settings.beginGroup("ProtocolTransferForm");

    settings.setValue("History_Count",ui->comboBox_HistoryFile->count());
    settings.setValue("History_File",ui->comboBox_HistoryFile->currentText());
    for(int i=0;i<ui->comboBox_HistoryFile->count();i++){
        settings.setValue(QString("History_File%1").arg(i),ui->comboBox_HistoryFile->itemText(i));
    }
    settings.setValue("cmb_SendMode",ui->cmb_SendMode->currentText());
    settings.endGroup();
}


void ProtocolTransferForm::onReadBytes(QByteArray bytes)
{
    mXmodem.onReadBytes(bytes);
}

void ProtocolTransferForm::onProtocolStateChange(STATE_CHANGE_TYPE_T type, int state)
{
    if(type == IOConnect_State){
        emit xmodemStateChange(Xmodem::IOConnectState,QString::number(state));
    }
}

void ProtocolTransferForm::onXmodemStateChange(Xmodem::XmodemState type, QString state)
{
    if(type == Xmodem::SendPercent){
        ui->progressBar->setValue(state.toFloat());
    }else if(type == Xmodem::SendInfo){
        showMsg("green",state);
    }else if(type == Xmodem::SendTransferState){
        if(state == "1"){
            showMsg("green","传输完成");
        }else if(state == QString("DisConnected")){
            showMsg("red","退出传输，通信IO连接失败！");
        }
        ui->comboBox_HistoryFile->setEnabled(true);
        ui->cmb_SendMode->setEnabled(true);
        ui->btn_selectFile->setEnabled(true);
        ui->btn_StartSend->setEnabled(true);
    }
}

void ProtocolTransferForm::on_btn_CleanWindow_clicked()
{
    ui->plainTextEdit->clear();
}


void ProtocolTransferForm::on_btn_selectFile_clicked()
{
    QString SelectFile = QFileDialog::getOpenFileName(
                this,                       // 父窗口
                tr("选择文件"),             // 标题
                lastDir,
                tr("所有文件 (*);;文本文件 (*.txt)")  // 过滤器
            );
    if(SelectFile.isEmpty()) return;
    ui->comboBox_HistoryFile->addItem(SelectFile);
    ui->comboBox_HistoryFile->setCurrentText(SelectFile);
    if(ui->comboBox_HistoryFile->count()>5){
        ui->comboBox_HistoryFile->removeItem(0);
    }
    lastDir = QFileInfo(ui->comboBox_HistoryFile->currentText()).absolutePath();
}


void ProtocolTransferForm::on_btn_StartSend_clicked()
{
    ProtocolMethod = ui->cmb_SendMode->currentText();
    SendFilePath   = ui->comboBox_HistoryFile->currentText();
    if(ProtocolMethod == "Xmodem 128" || ProtocolMethod == "Xmodem 1024")
    {
        mXmodem.StartSendXmodem(ProtocolMethod,SendFilePath);
    }

    ui->comboBox_HistoryFile->setEnabled(false);
    ui->cmb_SendMode->setEnabled(false);
    ui->btn_selectFile->setEnabled(false);
    ui->btn_StartSend->setEnabled(false);
}


void ProtocolTransferForm::on_btn_CancelSend_clicked()
{
    if(ProtocolMethod == "Xmodem 128" || ProtocolMethod == "Xmodem 1024")
    {
        mXmodem.CancelSendXmodem();
    }
    ui->comboBox_HistoryFile->setEnabled(true);
    ui->cmb_SendMode->setEnabled(true);
    ui->btn_selectFile->setEnabled(true);
    ui->btn_StartSend->setEnabled(true);
}



