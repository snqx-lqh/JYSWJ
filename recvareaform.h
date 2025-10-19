#ifndef RECVAREAFORM_H
#define RECVAREAFORM_H

#include <QWidget>
#include "terminal.h"
#include "common.h"
#include "waveshowform.h"

namespace Ui {
class RecvAreaForm;
}

class RecvAreaForm : public QWidget
{
    Q_OBJECT

public:
    explicit RecvAreaForm(QWidget *parent = nullptr);
    ~RecvAreaForm();

    void onStateChange(STATE_CHANGE_TYPE_T type,int state);
public slots:
    void onReadBytes(QByteArray bytes);
    void onAppendSendData(QByteArray bytes);
signals:
    void sendBytes(QByteArray bytes);

private:
    Ui::RecvAreaForm *ui;
    WaveShowForm mWaveShowForm;
};

#endif // RECVAREAFORM_H
