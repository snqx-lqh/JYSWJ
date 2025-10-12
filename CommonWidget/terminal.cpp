#include "terminal.h"
#include <QScrollBar>
#include <QMimeData>

Terminal::Terminal( QWidget *parent)
    : QPlainTextEdit(parent)
{
    setFont(QFont("Consolas", 10));
    //setStyleSheet("background:#222;color:#ddd;");
    setReadOnly(false);          // 允许光标到处跑，但所有输入走 io
    setContentsMargins(0,0,0,0);
    //setLineWrapMode(QPlainTextEdit::NoWrap);

    m_codec = QTextCodec::codecForName("UTF-8");
}

void Terminal::setIODevice(QIODevice *io)
{
    m_io  = io;
}

void Terminal::setEncoding(const QByteArray &encodingName)
{
    QTextCodec *codec = QTextCodec::codecForName(encodingName);
    if (codec)
        m_codec = codec;
}

void Terminal::onReadyRead()
{
    appendData(m_io->readAll());
}

void Terminal::appendData(const QByteArray &ba)
{
    QByteArray data = ba;
    data.replace("\r\n", "\n");
    //data.replace('\r', ' ');

    //
    QString text = m_codec ? m_codec->toUnicode(data) : QString::fromUtf8(data);

    QTextCursor cur = textCursor();
    cur.movePosition(QTextCursor::End);

    static int currentLineStartPos = 0;

    for (QChar  ch : text) {
        switch (ch.unicode()) {
            case '\b':
                if (cur.position() > currentLineStartPos)
                    cur.deletePreviousChar();
                break;
            case '\r':
                cur.setPosition(currentLineStartPos);
                break;
            case '\n':
                cur.insertText("\n");
                currentLineStartPos = cur.position();
                break;
            default:
                cur.insertText(ch);
                break;
        }
    }

    setTextCursor(cur);
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
        m_io->write(t.toUtf8());

    // 控制字符也直接发
    if (ev->modifiers() & Qt::ControlModifier) {
        char ch = 0;
        switch (ev->key()) {
        case Qt::Key_C: ch = 3;  break;
        case Qt::Key_Z: ch = 26; break;
        case Qt::Key_D: ch = 4;  break;
        default: return;
        }
        m_io->write(&ch, 1);
    }
}

void Terminal::insertFromMimeData(const QMimeData *source)
{
    if (source->hasText())
        m_io->write(source->text().toUtf8());
}
