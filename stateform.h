#ifndef STATEFORM_H
#define STATEFORM_H

#include <QWidget>
#include "common.h"

namespace Ui {
class StateForm;
}

class StateForm : public QWidget
{
    Q_OBJECT

public:
    explicit StateForm(QWidget *parent = nullptr);
    ~StateForm();

public slots:
    void onStateChange(STATE_CHANGE_TYPE_T type,int state);
    void onConnectInfo(QString Info);
private:
    Ui::StateForm *ui;
    quint64 sendCount = 0;
    quint64 recvCount = 0;

};

#endif // STATEFORM_H
