#include "customcombobox.h"
#include <QAction>
#include <QContextMenuEvent>
#include <QTextCodec>
#include <QListView>
#include <QMouseEvent>

CustomComboBox::CustomComboBox(QWidget *parent)
    : QComboBox(parent)
{
    // 控制大小策略（不自动扩展）
    setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);


    this->setToolTip(this->currentText());
    connect(this, &QComboBox::currentTextChanged,
            this, [this](const QString &text){
        this->setToolTip(text);
    });

    // 获取内部 view（下拉列表）
    QListView *view = qobject_cast<QListView *>(this->view());
    if (view)
    {
        // 安装事件过滤器拦截右键点击
        view->viewport()->installEventFilter(this);
    }
}

bool CustomComboBox::eventFilter(QObject *obj, QEvent *event)
{
    QListView *view = qobject_cast<QListView *>(this->view());
    if (obj == view->viewport() && event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        if (me->button() == Qt::RightButton)
        {
            QModelIndex index = view->indexAt(me->pos());
            if (index.isValid())
            {
                int row = index.row();
                QString item = this->itemText(row);
                auto ret = QMessageBox::question(this,
                                                 tr("删除确认"),
                                                 tr("是否删除当前项: \"%1\" ?").arg(item),
                                                 QMessageBox::Ok | QMessageBox::Cancel);
                if (ret == QMessageBox::Ok)
                    this->removeItem(row);
            }
            return true; // 拦截事件，不让原逻辑执行
        }
    }
    return QComboBox::eventFilter(obj, event);
}

