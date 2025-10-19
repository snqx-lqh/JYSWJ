#ifndef CUSTOMCOMBOBOX_H
#define CUSTOMCOMBOBOX_H

#include <QComboBox>
#include <QMenu>
#include <QContextMenuEvent>
#include <QTextCodec>
#include <QDebug>
#include <QDialog>
#include <QMessageBox>

class CustomComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit CustomComboBox(QWidget *parent = nullptr);
    bool eventFilter(QObject *obj, QEvent *event)override;
signals:

private:
    QModelIndex m_rightClickIndex;
};

#endif // CUSTOMCOMBOBOX_H
