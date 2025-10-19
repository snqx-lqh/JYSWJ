#include "waveshowform.h"
#include "ui_waveshowform.h"

WaveShowForm::WaveShowForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WaveShowForm)
{
    ui->setupUi(this);

    UiInit();
}

WaveShowForm::~WaveShowForm()
{
    delete ui;
}

void WaveShowForm::UiInit()
{
    ChannelEnable.resize(8);
    ChannelEnable.fill(true, 8);

    waveShow = new WaveShow(ui->widget_WaveShow);
    connect(waveShow,&WaveShow::vLineValue,this,[this](QStringList strList){
        WaveShowBtnItems[0].label->setText(strList[0]);
        WaveShowBtnItems[1].label->setText(strList[1]);
        WaveShowBtnItems[2].label->setText(strList[2]);
        WaveShowBtnItems[3].label->setText(strList[3]);
        WaveShowBtnItems[4].label->setText(strList[4]);
        WaveShowBtnItems[5].label->setText(strList[5]);
        WaveShowBtnItems[6].label->setText(strList[6]);
        WaveShowBtnItems[7].label->setText(strList[7]);
    });

    waveButtonInit();
}

void WaveShowForm::waveButtonInit()
{

    for(int i = 0;i < 8;i++)
    {
        WaveShowBtnItem item;
        item.btn       = new QPushButton(QString("CH%1").arg(i+1));
        item.label     = new QLabel("0");
        ui->gridLayout_WaveBtn->addWidget(item.btn,   0, i);
        ui->gridLayout_WaveBtn->addWidget(item.label, 1, i);
        switch (i) {
            case 0: item.color = "rgb(0, 121, 255)";   break;    // 科技蓝
            case 1: item.color = "rgb(120, 94, 240)";  break;    // 深紫蓝
            case 2: item.color = "rgb(255, 110, 176)"; break;    // 粉紫色
            case 3: item.color = "rgb(0, 212, 190)";   break;    // 薄荷绿
            case 4: item.color = "rgb(255, 153, 0)";   break;    // 琥珀橙
            case 5: item.color = "rgb(102, 187, 235)"; break;    // 浅天蓝
            case 6: item.color = "rgb(255, 87, 87)";   break;    // 珊瑚红
            case 7: item.color = "rgb(160, 214, 134)"; break;    // 嫩绿色
            default: break;
        }
        item.btn->setCursor(Qt::PointingHandCursor);
        item.btn->setStyleSheet(
            "QPushButton {" + QString("  background-color: %1;").arg(item.color) +
            "  border-radius:8px; border:1px solid #606060;padding:4px; "
            "}");
        item.label->setStyleSheet(
            "QLabel {" + QString("  background-color: %1;").arg(item.color) +
            "  border-radius:8px; border:1px solid #606060; padding:4px; "
            "}");
        connect(item.btn,&QPushButton::clicked,this,[this,item,i](){
            if(ChannelEnable[i] == false){
                ChannelEnable[i] = true;
                item.btn->setStyleSheet(
                    "QPushButton {" + QString("  background-color: %1;").arg(item.color) +
                    "  border-radius:8px; border:1px solid #606060;padding:4px; "
                    "}");
                item.label->setStyleSheet(
                    "QLabel {" + QString("  background-color: %1;").arg(item.color) +
                    "  border-radius:8px;border:1px solid #606060;padding:4px; "
                    "}");
            }else{
                ChannelEnable[i] = false;
                item.btn->setStyleSheet(
                    "QPushButton {"
                    "  background-color:rgb(255, 255, 255);"
                    "  border-radius:8px;border:1px solid #606060;padding:4px; "
                    "}");
                item.label->setStyleSheet(
                    "QLabel {"
                    "  background-color:rgb(255, 255, 255);"
                    "  border-radius:8px; border:1px solid #606060;padding:4px; "
                    "}");
            }
            waveShow->setLineVisible(i,ChannelEnable[i]);
        });

        WaveShowBtnItems.append(item);
    }
}

// 解析串口数据，提取逗号分隔的数字
// 在MainWindow类中实现的串口数据解析函数
QStringList WaveShowForm::parseSerialData(const QByteArray &raw)
{
    QStringList result;

    // 将QByteArray转换为QString以便处理
    QString data = QString::fromUtf8(raw);

    // 移除可能的回车换行符
    data = data.trimmed();

    // 检查输入是否为空
    if (data.isEmpty()) {
        return result;
    }

    // 查找*的位置，分离数据部分和校验位
    int starIndex = data.indexOf('*');
    QString dataPart;

    if (starIndex != -1) {
        // 提取*之前的部分
        dataPart = data.left(starIndex);
    } else {
        // 如果没有*，使用整个字符串
        dataPart = data;
    }

    // 按逗号分割数据
    QStringList parts = dataPart.split(',');

    // 过滤掉空字符串
    foreach (const QString &part, parts) {
        if (!part.isEmpty()) {
            result.append(part);
        }
    }

    return result;
}

void WaveShowForm::clearRecv()
{
    x_num = 0;
    waveShow->cleanData();
    for(int i = 0;i<8;i++)
    {
        WaveShowBtnItems[i].label->setText("0");
    }
}

// 读取串口数据后的处理
void WaveShowForm::onReadBytes(QByteArray bytes)
{
    //解析字符串到波形中
    if(bytes[0] == '$' && bytes[1] == 'W' && bytes[2] == 'A' && bytes[3] == 'V' && bytes[4] == 'E')
    {
        QStringList list = parseSerialData(bytes);
        for(int i = 0;i<list.count()-1;i++)
        {
            if(i > 7) continue;
            QString Y_DATA = list[i+1];
            waveShow->addData(i,x_num,Y_DATA.toDouble());
        }
        x_num++;
    }else{
        ;//
    }
}
