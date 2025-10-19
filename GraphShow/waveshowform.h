#ifndef WAVESHOWFORM_H
#define WAVESHOWFORM_H

#include <QWidget>
#include "qcustomplot.h"
#include "waveshow.h"


namespace Ui {
class WaveShowForm;
}

class WaveShowForm : public QWidget
{
    Q_OBJECT

public:
    explicit WaveShowForm(QWidget *parent = nullptr);
    ~WaveShowForm();

    void UiInit(void);
    void waveButtonInit(void);
    QStringList parseSerialData(const QByteArray &raw);
    void clearRecv(void);

    struct WaveShowBtnItem
    {
        QPushButton *btn;
        QLabel      *label;
        QString      color;
    };

public slots:
    void onReadBytes(QByteArray bytes);

private:
    Ui::WaveShowForm *ui;
    QVector<WaveShowBtnItem> WaveShowBtnItems;
    WaveShow *waveShow;
    QVector<bool> ChannelEnable;
    qint32   x_num=0;
};

#endif // WAVESHOWFORM_H
