#include "terminal.h"
#include <QScrollBar>
#include <QMimeData>

Terminal::Terminal( QWidget *parent)
    : QPlainTextEdit(parent)
{
    setFont(QFont("Consolas", 10));
    setStyleSheet("background:#222;color:#ddd;");
    //setReadOnly(true);          // 允许光标到处跑，但所有输入走 io
    setContentsMargins(0,0,0,0);

    m_codec = QTextCodec::codecForName("UTF-8");
    m_fixedCursor = textCursor();
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

//    if(m_expectUpEcho || m_expectDownEcho || m_expectRightEcho
//            || m_expectLeftEcho || m_expectEnterEcho){
//    }else{
//        cur.movePosition(QTextCursor::End);
//    }

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

    for (int i = 0; i < text.size(); ++i) {
        QChar ch = text.at(i);
        ushort code = ch.unicode();

        // --- 判断是否为控制字符 ---
        bool isControlChar = (code < 0x20) || (code == 0x7F);

        if (isControlChar) {
            switch (code) {
            case '\b': // Backspace
                if (cur.positionInBlock() > 0)
                    cur.movePosition(QTextCursor::Left);
                if(m_expectUpEcho) {
                    keyState = KEY_UP_STATE;
                }else if(m_expectDownEcho){
                    keyState = KEY_DOWN_STATE;
                }else if(m_expectLeftEcho){
                    keyState = KEY_LEFT_STATE;
                    m_expectLeftEcho = false;
                }else{
                    keyState = KEY_BACK_STATE;
                }
                break;
            case '\r': // 回车
                // 移到行首
                cur.movePosition(QTextCursor::StartOfBlock);
                break;
            case '\n': // 换行
                cur.movePosition(QTextCursor::EndOfBlock);
                cur.insertBlock();
                break;
            case 0x0C:
                clear();
                break;
            default:
                // 其他控制字符暂不处理
                break;
            }
        } else {
            // --- 普通可打印字符 ---
            if (keyState == KEY_BACK_STATE) {
                // 如果上一个是退格+空格组合，擦除并重写
                cur.deleteChar();
                if(ch == 0x20){
                    keyState = KEY_NORMAL_STATE;
                }
            }

            if (keyState == KEY_UP_STATE || keyState == KEY_DOWN_STATE) {
                if(ch == 0x20){
                    cur.deleteChar();
                }else{
                    m_expectUpEcho   = false;
                    m_expectDownEcho = false;
                    keyState = KEY_NORMAL_STATE;
                }
            }
            if(m_expectRightEcho){
                cur.deleteChar();
                m_expectRightEcho = false;
            }
            cur.insertText(ch);
        }
    }
    setTextCursor(cur);
    ensureCursorVisible();
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
        cur.insertText(text);
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
    QByteArray bytes;

    switch (ev->key()) {
    case Qt::Key_Up:        bytes = "\x1B[A"; m_expectUpEcho    = true;break;
    case Qt::Key_Down:      bytes = "\x1B[B"; m_expectDownEcho  = true;break;
    case Qt::Key_Right:     bytes = "\x1B[C"; m_expectRightEcho = true;break;
    case Qt::Key_Left:      bytes = "\x1B[D"; m_expectLeftEcho  = true;break;
    case Qt::Key_Backspace: bytes = QByteArray(1, '\b');m_expectEnterEcho = true;break;  // 有些系统是 '\b'
    case Qt::Key_Tab:       bytes = QByteArray(1, '\t'); break;    // Tab
    case Qt::Key_Return:
    case Qt::Key_Enter:     bytes = QByteArray(1, '\r'); break;    // 回车
    default:
        // 普通字符
        if (!ev->text().isEmpty())
            bytes = ev->text().toUtf8();
        break;
    }

    // Ctrl 组合键
    if (ev->modifiers() & Qt::ControlModifier) {
        switch (ev->key()) {
        case Qt::Key_C: bytes = QByteArray(1, 3); break;   // Ctrl+C
        case Qt::Key_Z: bytes = QByteArray(1, 26); break;  // Ctrl+Z
        case Qt::Key_D: bytes = QByteArray(1, 4); break;   // Ctrl+D
        default: break;
        }
    }

    if (!bytes.isEmpty())
        emit sendBytes(bytes);

}

void Terminal::insertFromMimeData(const QMimeData *source)
{
    if (source->hasText())
        emit sendBytes(source->text().toUtf8());
}


void Terminal::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        int pos = cursorForPosition(e->pos()).position();
        m_customSelection.start = m_customSelection.end = pos;
        m_customSelection.active = true;
        viewport()->update();          // 触发重绘
    }
}

void Terminal::mouseMoveEvent(QMouseEvent *e)
{
    if (!m_customSelection.active) return;
    m_customSelection.end = cursorForPosition(e->pos()).position();
    viewport()->update();
}

void Terminal::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_customSelection.active = false;
        viewport()->update();
    }
}

void Terminal::paintEvent(QPaintEvent *e)
{
    // 先让 Qt 画完文本、光标、行号等
    QPlainTextEdit::paintEvent(e);

    if (m_customSelection.start < 0 || m_customSelection.end < 0) return;

    // 保证 start <= end
    int start = qMin(m_customSelection.start, m_customSelection.end);
    int end = qMax(m_customSelection.start, m_customSelection.end);

    QPainter p(viewport());
    p.setBrush(QColor(0,120,215,80));   // 半透明蓝色
    p.setPen(Qt::NoPen);

    // 把字符范围换算成矩形列表
    QTextCursor cur(document());
    cur.setPosition(start);
    QTextCursor curE(document());
    curE.setPosition(end);

    QRect r1 = cursorRect(cur);
    QRect r2 = cursorRect(curE);

    if (r1.top() == r2.top()) {          // 同一行
        p.drawRect(r1.left(), r1.top(), r2.right() - r1.left(), r1.height());
    } else {                             // 多行
        // 首行剩余部分
        p.drawRect(r1.left(), r1.top(), viewport()->width() - r1.left(), r1.height());
        // 中间完整行
        for (int block = cur.blockNumber() + 1; block < curE.blockNumber(); ++block)
            p.drawRect(0, cursorRect(QTextCursor(document()->findBlockByNumber(block))).top(),
                       viewport()->width(), fontMetrics().height());
        // 末行开头部分
        p.drawRect(0, r2.top(), r2.right(), r2.height());
    }
}

void Terminal::copy()
{
    qDebug()<<"in copy";
    if (m_customSelection.start < 0 || m_customSelection.end < 0) return;
    int s = qMin(m_customSelection.start, m_customSelection.end);
    int e = qMax(m_customSelection.start, m_customSelection.end);
    QTextCursor cur(document());
    cur.setPosition(s);
    cur.setPosition(e, QTextCursor::KeepAnchor);
    QApplication::clipboard()->setText(cur.selectedText());
}

void Terminal::paste()
{
    QString text = QApplication::clipboard()->text();
    if (text.isEmpty()) return;

    // 在“真实光标”处插入
//    QTextCursor cur = m_fixedCursor;   // 你之前保存的“禁止移动”光标
//    cur.insertText(text);
//    setTextCursor(cur);                // 更新光标位置到插入之后
//    m_fixedCursor = cur;               // 记录新位置

    const QMimeData *md = QApplication::clipboard()->mimeData();
    insertFromMimeData(md);           // ← 最终只会调这里
}

void Terminal::contextMenuEvent(QContextMenuEvent *e)
{
    Q_UNUSED(e);
    QMenu menu(this);
    menu.addAction("复制", this, &Terminal::copy);
    menu.addAction("粘贴", this, &Terminal::paste);
    menu.exec(QCursor::pos());
}



