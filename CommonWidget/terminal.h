#ifndef TERMINAL_H
#define TERMINAL_H

#include <QPlainTextEdit>
#include <QIODevice>
#include <QDebug>
#include <QTextCodec>
#include <QDateTime>
#include <QPainter>
#include <QMenu>
#include <QClipboard>
#include <QApplication>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextBlock>

class Terminal : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit Terminal(  QWidget *parent = nullptr);

    enum KeyState{
        KEY_NORMAL_STATE = 0,
        KEY_UP_STATE,
        KEY_DOWN_STATE,
        KEY_LEFT_STATE,
        KEY_RIGHT_STATE,
        KEY_BACK_STATE
    };
    Q_ENUM(KeyState)


    void setEncoding(const QByteArray &encodingName);   // 新增：设置编码
    void appendData(const QByteArray &ba);
    void appendSendData(const QByteArray &ba);
    void setShowHexState(bool state);
    void setShowDateState(bool state);

    // 重写选择相关方法
    void setSelection(int start, int end);
    void clearSelection();
    QString getSelectedText() const;
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *event)override;
    void mouseMoveEvent(QMouseEvent *event)override;
    void mouseReleaseEvent(QMouseEvent *event)override;
    void contextMenuEvent(QContextMenuEvent *e) override;  // 新增：右键菜单事件
    void copy(void);
    void paste(void);

private:
    QTextCodec *m_codec = nullptr;                      // 当前使用的编码
    bool mShowHexState  = false;
    bool mShowDateState = false;

    // 当前输入行缓冲（远端交互行）
    QString m_lineBuffer;      // 当前行内容（未包含末尾换行）
    int     m_lineCursor = 0;  // 光标在 m_lineBuffer 中的位置（0..len）

    // 文档中当前输入行起始位置（文档内部字符索引）
    int     m_lineStartPos = 0; // 以 QTextDocument 的字符索引为准

    // 当本地发送方向键时，期望下位机回显（以便区分“回显字符”与“插入字符”）
    bool    m_expectRightEcho  = false;
    bool    m_expectLeftEcho   = false;
    bool    m_expectUpEcho     = false;
    bool    m_expectDownEcho   = false;
    bool    m_expectEnterEcho  = false;

    KeyState keyState = KEY_NORMAL_STATE;

     struct CustomSelection {
         int start;
         int end;
         bool active;
     };

     CustomSelection m_customSelection;
     QTextCursor m_fixedCursor;  // 固定的输入光标
     bool m_isSelecting    = false;
     int m_selectionAnchor = 0;

signals:
    void sendBytes(QByteArray bytes);

public slots:
    void onReadBytes(QByteArray bytes);

protected:
    void keyPressEvent(QKeyEvent *ev) override;
    void insertFromMimeData(const QMimeData *source) override;

private slots:

};

#endif // TERMINAL_H
