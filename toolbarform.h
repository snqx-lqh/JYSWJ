#ifndef TOOLBARFORM_H
#define TOOLBARFORM_H

#include <QWidget>
#include <QIcon>
#include "common.h"
#include <QDebug>
#include <QSettings>
#include <QDir>

namespace Ui {
class ToolBarForm;
}

class ToolBarForm : public QWidget
{
    Q_OBJECT

public:
    explicit ToolBarForm(QWidget *parent = nullptr);
    ~ToolBarForm();

    void loadSettings();
    void saveSettings();
    void stateInit();

public slots:
    void onStateChange(STATE_CHANGE_TYPE_T type,int state);

private slots:
    void on_pushButton_Connect_clicked();

    void on_pushButton_ZK_clicked();

    void on_pushButton_MultiSend_clicked();

signals:
    void stateChange(STATE_CHANGE_TYPE_T type,int state);

private:
    Ui::ToolBarForm *ui;

    QIcon iconExpand,iconConnect,iconDisconnect,iconMultiSend;
    bool  connectState = false;

    bool expandState = true;
    bool multiSendState = true;
    QString         mIniFile;
};

#endif // TOOLBARFORM_H
