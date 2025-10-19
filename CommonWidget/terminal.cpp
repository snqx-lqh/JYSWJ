#include "terminal.h"
#include <QScrollBar>
#include <QMimeData>

Terminal::Terminal( QWidget *parent)
    : QPlainTextEdit(parent)
{
    setFont(QFont("Consolas", 10));
    setStyleSheet("background:#222;color:#ddd;");
    setReadOnly(false);          // 允许光标到处跑，但所有输入走 io
    setContentsMargins(0,0,0,0);
    //setLineWrapMode(QPlainTextEdit::NoWrap);

    m_codec = QTextCodec::codecForName("UTF-8");
}


void Terminal::setEncoding(const QByteArray &encodingName)
{
    QTextCodec *codec = QTextCodec::codecForName(encodingName);
    if (codec)
        m_codec = codec;
}


void Terminal::appendData(const QByteArray &ba)
{
    if (ba.isEmpty()) return;

    // 使用常量引用，避免不必要的复制
    const QByteArray &data = ba;
    QTextCursor cur = textCursor();
    cur.movePosition(QTextCursor::End);

    // ===== 时间戳显示 =====
    if (mShowDateState) {
        QString dateTime = QString("[%1] 收<- ")
                .arg(QDateTime::currentDateTime().toString("yy-MM-dd hh:mm:ss.zzz"));
        cur.insertText("\n" + dateTime);
    }

    // ===== HEX 模式 =====
    if (mShowHexState) {
        QString hexText = data.toHex(' ');
        cur.insertText(hexText + ' ');
        setTextCursor(cur);
        return;
    }

    // ===== 文本模式 =====
    QString text = m_codec ? m_codec->toUnicode(data) : QString::fromUtf8(data);
    // 智能处理 UBoot 输出中的 \r\n\r
    text.replace("\n\r", "\n");   // 先处理 0D 0A 0D 这种典型情况

    // 这里可选地手动处理退格符（因为 Qt 不会自动识别 \b）
    QString processed;
    processed.reserve(text.size());
    for (QChar ch : text) {
        if (ch == QChar('\b')) {
            if (!processed.isEmpty())
                processed.chop(1);
        } else {
            processed.append(ch);
        }
    }

    // 一次性插入，提高性能
    cur.insertText(processed);
    setTextCursor(cur);
}


void Terminal::appendSendData(const QByteArray &ba)
{
    QByteArray data = ba;
    QString text = m_codec ? m_codec->toUnicode(data) : QString::fromUtf8(data);

    QTextCursor cur = textCursor();
    cur.movePosition(QTextCursor::End);

    if(mShowDateState){
        QString dateTime = '\n' + QDateTime::currentDateTime()
                .toString("[yy-MM-dd hh:mm:ss.zzz]发->");
        cur.insertText(dateTime);
    }

    if(mShowHexState){
        QString hexShowTemp = data.toHex(' ')+" ";
        cur.insertText(hexShowTemp);
    }else{
        cur.insertText(data);
    }

    setTextCursor(cur);
}

void Terminal::setShowHexState(bool state)
{
    mShowHexState = state;
}

void Terminal::setShowDateState(bool state)
{
    mShowDateState = state;
}

void Terminal::onReadBytes(QByteArray bytes)
{
    appendData(bytes);
}

void Terminal::keyPressEvent(QKeyEvent *ev)
{
    // 只把字符写给 IO，本地不做任何插入
    QString t = ev->text();
    if (!t.isEmpty())
        sendBytes(t.toUtf8());

    // 控制字符也直接发
    if (ev->modifiers() & Qt::ControlModifier) {
        char ch = 0;
        switch (ev->key()) {
        case Qt::Key_C: ch = 3;  break;
        case Qt::Key_Z: ch = 26; break;
        case Qt::Key_D: ch = 4;  break;
        default: return;
        }
        QByteArray bytes;
        bytes.append(ch);
        sendBytes(bytes);
    }
}

void Terminal::insertFromMimeData(const QMimeData *source)
{
    if (source->hasText())
        sendBytes(source->text().toUtf8());
}
