#ifndef SERIALSETTINGSFORM_H
#define SERIALSETTINGSFORM_H

#include <QWidget>

#include <QDialog>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QAbstractNativeEventFilter>
#include <Windows.h>
#include <dbt.h>
#include <QList>

namespace Ui {
class SerialSettingsForm;
}

class SerialSettingsForm : public QDialog,public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    explicit SerialSettingsForm(QSerialPort *serialPort, QWidget *parent = nullptr);
    ~SerialSettingsForm();

    void scanSerialPorts();

    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);

    void addBaudItems();

public slots:
    void onUpdateSerialInfo();

private slots:

    void on_btn_OK_clicked();

    void on_btn_Cancel_clicked();



private:
    Ui::SerialSettingsForm *ui;

    QSerialPort *mSerialPort;
};

#endif // SERIALSETTINGSFORM_H
