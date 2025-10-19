#include "fileconvertform.h"
#include "ui_fileconvertform.h"

FileConvertForm::FileConvertForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileConvertForm)
{
    ui->setupUi(this);
    QDir dir(QCoreApplication::applicationDirPath());
    m_iniFile = dir.filePath("Config/settings.ini");

    ui->comboBox_ProtocolSelect->addItem("IMX6ULL_256_IMX");
    ui->comboBox_ProtocolSelect->addItem("IMX6ULL_512_IMX");

    loadSettings();
}

FileConvertForm::~FileConvertForm()
{
    saveSettings();
    delete ui;
}

void FileConvertForm::loadSettings()
{
    QSettings settings(m_iniFile,QSettings::IniFormat);
    settings.beginGroup("FileConvertForm");

    int History_Count = settings.value("History_Count",0).toInt();

    for(int i=0;i<History_Count;i++){
        QString temp = settings.value(QString("History_File%1").arg(i)).toString();
        bool exists = QFileInfo::exists(temp);
        if(exists)
            ui->comboBox_HistoryFile->addItem(temp);
    }
    ui->comboBox_ProtocolSelect->setCurrentText(settings.value("comboBox_ProtocolSelect",ui->comboBox_ProtocolSelect->currentText()).toString());
    ui->comboBox_HistoryFile->setCurrentText(settings.value("History_File",ui->comboBox_HistoryFile->currentText()).toString());
    settings.endGroup();
}

void FileConvertForm::saveSettings()
{
    QSettings settings(m_iniFile,QSettings::IniFormat);
    settings.beginGroup("FileConvertForm");

    settings.setValue("History_Count",ui->comboBox_HistoryFile->count());
    settings.setValue("History_File",ui->comboBox_HistoryFile->currentText());
    for(int i=0;i<ui->comboBox_HistoryFile->count();i++){
        settings.setValue(QString("History_File%1").arg(i),ui->comboBox_HistoryFile->itemText(i));
    }
    settings.setValue("comboBox_ProtocolSelect",ui->comboBox_ProtocolSelect->currentText());
    settings.endGroup();
}

void FileConvertForm::on_pushButton_Clear_clicked()
{
    ui->plainTextEdit->clear();
}

void FileConvertForm::showMsg(QString color, QString msg)
{
    QString timeStr = QDateTime::currentDateTime().toString("[yy-MM-dd hh:mm:ss]");
    msg = timeStr + msg;

    QString html = QStringLiteral("<font color=\"%1\">%2</font>")
                   .arg(color.toHtmlEscaped(), msg.toHtmlEscaped());
    ui->plainTextEdit->appendHtml(html);
}

void FileConvertForm::on_pushButton_FileConvert_clicked()
{
    QFile fileConvert;
    QFile fileNow;
    QByteArray fileTemp;
    if(ui->comboBox_ProtocolSelect->currentText() == "IMX6ULL_512_IMX"){
        for (uint32_t v : imx6_512mb_ivtdcd_table) {
            fileTemp.append(reinterpret_cast<const char*>(&v), sizeof(uint32_t));
        }
    }else if(ui->comboBox_ProtocolSelect->currentText() == "IMX6ULL_256_IMX"){
        for (uint32_t v : imx6_256mb_ivtdcd_table) {
            fileTemp.append(reinterpret_cast<const char*>(&v), sizeof(uint32_t));
        }
    }
    fileTemp.append(QByteArray(2048,'\0'));
    fileNow.setFileName(ui->comboBox_HistoryFile->currentText());
    if(fileNow.open(QIODevice::ReadOnly)){
        QByteArray temp = fileNow.readAll();
        fileTemp.append(temp);
    }

    fileConvert.setFileName(ui->comboBox_HistoryFile->currentText().replace(".bin",".imx"));
    if(fileConvert.open(QIODevice::ReadWrite)){
        fileConvert.write(fileTemp);
    }
    showMsg("green",QString("文件转换完成"));
}


void FileConvertForm::on_pushButton_FileSelect_clicked()
{
    QString SelectFile = QFileDialog::getOpenFileName(
                this,                       // 父窗口
                tr("选择文件"),             // 标题
                lastDir,
                tr("所有文件 (*);;文本文件 (*.txt)")  // 过滤器
            );
    ui->comboBox_HistoryFile->addItem(SelectFile);
    ui->comboBox_HistoryFile->setCurrentText(SelectFile);
    if(ui->comboBox_HistoryFile->count()>5){
        ui->comboBox_HistoryFile->removeItem(0);
    }
    lastDir = QFileInfo(SelectFile).absolutePath();
}

