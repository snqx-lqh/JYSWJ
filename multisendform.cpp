#include "multisendform.h"
#include "ui_multisendform.h"

MultiSendForm::MultiSendForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MultiSendForm)
{
    ui->setupUi(this);

    QDir dir(QCoreApplication::applicationDirPath());
    mIniFile = dir.filePath("Config/多字符配置文件/default.ini");
    mIniFileSettings = dir.filePath("Config/settings.ini");
    UiInit();
    loadSettings();

    QString CurrentIni;
    QSettings settings(mIniFileSettings,QSettings::IniFormat);
    settings.beginGroup("MultiSendFormSettings");
    ui->checkBox_SendNewLine->setChecked(settings.value("checkBox_SendNewLine","false").toBool());
    CurrentIni = settings.value("comboBox_IniFile").toString();
    settings.endGroup();

    ui->comboBox_IniFile->addItem("default.ini");

    QString iniFilePath = dir.filePath("Config/多字符配置文件");
    QFileInfoList list = QDir(iniFilePath).entryInfoList(QStringList()
                         << "*.ini", QDir::Files);
    for(int i = 0;i < list.count();i++){
        if(ui->comboBox_IniFile->findText(list[i].fileName()) == -1){
            ui->comboBox_IniFile->addItem(list[i].fileName());
        }
    }
    ui->comboBox_IniFile->setCurrentText("default.ini");

    bool exists = QFileInfo::exists(mIniFileSettings);
    if(!CurrentIni.isEmpty() && exists){
        ui->comboBox_IniFile->setCurrentText(CurrentIni);
        QString temp = CurrentIni;
        QDir dir(QCoreApplication::applicationDirPath());
        mIniFile = dir.filePath("Config/多字符配置文件/")+temp;
        loadSettings();
    }

    MultiSendCycleTimer = new QTimer;
    connect(MultiSendCycleTimer,SIGNAL(timeout()),this,SLOT(onMultiSendCycleTimerOut()));

    initFinished = true;
}

MultiSendForm::~MultiSendForm()
{
    saveSettings();

    QSettings settings(mIniFileSettings,QSettings::IniFormat);
    settings.beginGroup("MultiSendFormSettings");
    settings.setValue(QString("checkBox_SendNewLine"),ui->checkBox_SendNewLine->isChecked());
    settings.setValue(QString("comboBox_IniFile"),ui->comboBox_IniFile->currentText());
    settings.endGroup();

    delete ui;
}



void MultiSendForm::UiInit()
{
    for (int i = 0; i < mulSendItemMaxNum; ++i) {
        MultiSendItem item;

        item.itemIndex = i;
        item.check = new QCheckBox();
        item.edit = new QLineEdit();
        item.btn = new QPushButton(QString("%1").arg(i));
        item.btn->setProperty("row", i);
        item.indexEdit = new QLineEdit(QString("0"));
        item.indexEdit->setMaximumWidth(20);
        item.timerEdit = new QLineEdit(QString("1000"));
        item.timerEdit->setMaximumWidth(40);
        ui->gridLayout_MultiItem->addWidget(item.check,     i ,   0);
        ui->gridLayout_MultiItem->addWidget(item.edit,      i ,   1);
        ui->gridLayout_MultiItem->addWidget(item.btn,       i ,   2);
        ui->gridLayout_MultiItem->addWidget(item.indexEdit, i ,   3);
        ui->gridLayout_MultiItem->addWidget(item.timerEdit, i ,   4);

        connect(item.btn, &QPushButton::clicked, this, [this, item]() {
            sendText(item);
        });

        MultiSendItems.append(item);
    }


}

void MultiSendForm::loadSettings()
{
    QSettings settings(mIniFile,QSettings::IniFormat);
    settings.beginGroup("MultiSendForm");

    for (int i = 0; i < MultiSendItems.size() && i < mulSendItemMaxNum; ++i) {
        auto &it = MultiSendItems[i];
        it.check->setChecked(settings.value(QString("%1/check").arg(i), false).toBool());
        it.edit->setText(settings.value(QString("%1/edit").arg(i), "").toString());
        it.btn->setText(settings.value(QString("%1/btn").arg(i), it.btn->text()).toString());
        it.indexEdit->setText(settings.value(QString("%1/indexEdit").arg(i), 0).toString());
        it.timerEdit->setText(settings.value(QString("%1/timerEdit").arg(i), 1000).toString());
    }

    settings.endGroup();
}

void MultiSendForm::saveSettings()
{
    QSettings settings(mIniFile,QSettings::IniFormat);
    settings.beginGroup("MultiSendForm");

    for (int i = 0; i < MultiSendItems.size(); ++i) {
        const auto &it = MultiSendItems[i];
        settings.setValue(QString("%1/check").arg(i),      it.check->isChecked());
        settings.setValue(QString("%1/edit").arg(i),       it.edit->text());
        settings.setValue(QString("%1/btn").arg(i),        it.btn->text());
        settings.setValue(QString("%1/indexEdit").arg(i),  it.indexEdit->text().toInt());
        settings.setValue(QString("%1/timerEdit").arg(i),  it.timerEdit->text().toInt());
    }
    settings.endGroup();
}


void MultiSendForm::sendText(MultiSendItem item)
{
    if(connectState == false){
        QMessageBox::warning(nullptr,"警告","通信IO未连接");
        return;
    }
    QString text = item.edit->text();
    text.replace("\\r","\r").replace("\\n","\n");
    if (item.check->isChecked()) {
        // 1. 按十六进制发送
        QByteArray bytes = QByteArray::fromHex(
                               text.toUtf8().simplified());   // 去掉空格、回车
        if(ui->checkBox_SendNewLine->isChecked()) {
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
        if(ui->checkBox_SendNewLine->isChecked()) {
            bytes.append(0x0D);
            bytes.append(0x0A);
        }
        emit sendBytes(bytes);
    }
}

void MultiSendForm::onMultiSendCycleTimerOut()
{
    if(connectState == false){
        QMessageBox::warning(nullptr,"警告","通信IO未连接");
        ui->checkBox_CycleSend->setChecked(false);
        MultiSendCycleTimer->stop();
        return;
    }
    if (!txQueue.isEmpty())
    {
        MultiSendItem temp;
        temp = txQueue.at(txQueueGetIndex);
        txQueueGetIndex++;
        if(txQueueGetIndex>=txQueue.length())
            txQueueGetIndex=0;
        sendText(temp);
        MultiSendCycleTimer->start(temp.timerEdit->text().toUInt());
    }
}

void MultiSendForm::onStateChange(STATE_CHANGE_TYPE_T type, int state)
{
    if(type == IOConnect_State){
        if(state == 0) connectState = false;
        else connectState = true;
    }else if(type == IOSendNewLine_State){
        if(state == 0) sendNewLineState = false;
        else sendNewLineState = true;
    }else if(type == TextCodec){
        if(state == 0) TextCodeC = "GBK";
        else if(state == 1) TextCodeC = "UTF-8";
    }
}


void MultiSendForm::on_pushButton_NewIni_clicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("新建INI文件"),
                                         tr("新建INI文件(不带后缀):"), QLineEdit::Normal,
                                         "", &ok);
    if (ok && !text.isEmpty())
    {
        QString iniTemp = text+".ini";
        if(ui->comboBox_IniFile->findText(iniTemp) == -1){
            ui->comboBox_IniFile->addItem(text+".ini");
        }else{
            QMessageBox::warning(nullptr,"文件提示",QString(text+".ini 已存在"));
            return;
        }
    }
}


void MultiSendForm::on_comboBox_IniFile_currentTextChanged(const QString &arg1)
{
    if(initFinished){
        saveSettings();
        QString temp = arg1;
        QDir dir(QCoreApplication::applicationDirPath());
        mIniFile = dir.filePath("Config/多字符配置文件/")+temp;
        loadSettings();
    }
}


void MultiSendForm::on_checkBox_CycleSend_stateChanged(int arg1)
{
    if(2 == arg1){
        if(connectState == false){
            QMessageBox::warning(nullptr,"警告","通信IO未连接");
            ui->checkBox_CycleSend->setChecked(false);
            return;
        }
        txQueue.clear();
        txQueueGetIndex = 0;
        MultiSendItemsTemp = MultiSendItems;     // 20 条记录已经填好
        std::sort(MultiSendItemsTemp.begin(), MultiSendItemsTemp.end(),
          [](const MultiSendItem &a, const MultiSendItem &b)
          {
              if (a.indexEdit->text().toUInt() != b.indexEdit->text().toUInt())
                  return a.indexEdit->text().toUInt() < b.indexEdit->text().toUInt();      // 次序小在前
              return a.itemIndex < b.itemIndex;      // 同次序按序列号
          });
        for (const MultiSendItem &it : qAsConst(MultiSendItemsTemp))
        {
            if(it.indexEdit->text().toUInt()!=0) txQueue.enqueue(it);
        }
        MultiSendCycleTimer->start();
    }else{
        // 未勾选
        MultiSendCycleTimer->stop();
    }
}

